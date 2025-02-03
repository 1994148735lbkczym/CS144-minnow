#include "tcp_sender.hh"
#include "debug.hh"
#include "tcp_config.hh"
#include "cmath"

using namespace std;

// This function is for testing only; don't add extra state to support it.
uint64_t TCPSender::sequence_numbers_in_flight() const
{
  return last_segment_sent_ - last_ackno_received;
}

// This function is for testing only; don't add extra state to support it.
uint64_t TCPSender::consecutive_retransmissions() const
{
  return log2( curr_RTO_ms_ / initial_RTO_ms_);
}

void TCPSender::push_packet(uint64_t size, TCPSenderMessage &cur_message, const TransmitFunction& transmit, bool rst, bool can_add_fin_bit) {
  string_view bytestream_state = input_.reader().peek();
  
  cur_message.SYN = last_segment_sent_ == 0;
  if (cur_message.SYN && size != 0) {
    size--;
  }
  
  // seqno -> bytes popped from stream
  cur_message.seqno = Wrap32::wrap(last_segment_sent_, isn_);
  if (can_add_fin_bit) {
    cur_message.FIN = true;
    sent_fin_bit = true;
    size = min(size, size - 1);
  }
  cur_message.payload = bytestream_state.substr(0, size);
  input_.reader().pop(size);
  
  cur_message.RST = rst;
  transmit(cur_message);

  // if (!false_window) {
    // add to dictionary - assuming passing by value here
    outstanding_segments[last_segment_sent_] = cur_message;
    last_segment_sent_ += cur_message.sequence_length();
    bytes_outstanding += cur_message.sequence_length();
    if (!rto_timer_is_on_) {
      set_rto_alarm();
    }
  // }
}

void TCPSender::push( const TransmitFunction& transmit )
{
  TCPSenderMessage cur_message;

  // check for RST Flag -> push RST packet
  if (RST || input_.has_error()) {
    push_packet(0, cur_message, transmit, RST || input_.has_error(), false);
    return;
  }

  // send just SYN Flag if sender is reaching out first
  if ( input_.reader().bytes_buffered() == 0 && !connection_started ) {
    push_packet(0, cur_message, transmit, false, input_.writer().is_closed());
    return;
  }

  //size_t empty_message_size = cur_message.sequence_length();
  // if window_size is 0 and this is not the first message, always send a packet of size one to recieve new window size information
  if (window_size == 0 && connection_started) {
     if (!bytes_outstanding) {
      push_packet(1, cur_message, transmit, false, input_.reader().is_finished());
     }
    return;
  }

  // otherwise, transmit as big of a message as you can
  size_t curr_space_in_connection = window_size - bytes_outstanding; // 7 bytes of non-payload info
  size_t payload_size = min(min(TCPConfig::MAX_PAYLOAD_SIZE, curr_space_in_connection), input_.reader().bytes_buffered());
  bool can_add_syn_bit = last_segment_sent_ == 0 && curr_space_in_connection - payload_size;
  //payload_size += can_add_syn_bit;
  bool can_add_fin_bit = (input_.writer().is_closed() && payload_size >= input_.reader().bytes_buffered() && !sent_fin_bit); // can add if BS is closed + all bytes will fit
  can_add_fin_bit = (can_add_fin_bit && curr_space_in_connection - payload_size); // Check for space in window
  //payload_size += can_add_fin_bit;
  

  // assuming window_size is bytes free in the bytestream; if less then 7 bytes if available cannot create a TCPMessage
  // TODO: figure out how to add FIN/SYN flag status to input.reader().size
  while (curr_space_in_connection > 0 && payload_size + (can_add_fin_bit && !sent_fin_bit) > 0) {
    // Re-evaluate the vars that could change after a packet is pushed
    can_add_fin_bit = (input_.writer().is_closed() && payload_size >= input_.reader().bytes_buffered() && !sent_fin_bit); // can add if BS is closed + all bytes will fit
    can_add_fin_bit = (can_add_fin_bit && curr_space_in_connection - payload_size);
    size_t overall_size = payload_size + can_add_fin_bit + can_add_syn_bit;
    push_packet(overall_size, cur_message, transmit, false, can_add_fin_bit);
    // Re-evaluate the vars that could change after a packet is pushed
    curr_space_in_connection = window_size - bytes_outstanding;
    payload_size = min(min(TCPConfig::MAX_PAYLOAD_SIZE, curr_space_in_connection), input_.reader().bytes_buffered());
    can_add_fin_bit = (input_.writer().is_closed() && input_.reader().bytes_buffered() < curr_space_in_connection && !sent_fin_bit);
  }
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  TCPSenderMessage cur_message;
  cur_message.seqno = Wrap32::wrap(last_segment_sent_, isn_);
  cur_message.RST = RST || input_.has_error();
  return cur_message;
}

/*
Goal of this function is to find the ackno of the first byte of the first, unacknowledged segment.
The first unackno. segment is always the key before, a key k s.t. k > msg_ackno.
*/
void TCPSender::update_last_segment_recieved_and_clean(uint64_t msg_ackno) {
  auto it = outstanding_segments.begin();
  while (it != outstanding_segments.end()) {
    if (it->first >= msg_ackno) {
        break;
    }
    // decrement bytes_outstanding and clean map if we have ackno'd all the bytes
    if (it->first + it->second.sequence_length() <= msg_ackno) {
      bytes_outstanding = last_segment_sent_ - msg_ackno;
      last_segment_recieved_ += it->second.sequence_length();
      it = outstanding_segments.erase(it);  // erase returns next valid iterator
    } else {
      // Update Outstanding Bytes for a partial segment recognition
      bytes_outstanding = last_segment_sent_ - msg_ackno;
      ++it;
    }
  }
  // decrement bytes_outstanding
  bytes_outstanding = last_segment_sent_ - msg_ackno;

}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  RST = msg.RST || input_.has_error();
  window_size = msg.window_size;
  if (RST) {
    input_.set_error();
  }
  
  // if the msg has no value, skip it
  if (!msg.ackno.has_value()) {
    return;
  }

  // TODO: Clean collection of outstanding segments; add below to a helper function under RTO + decrement bytes_outstanding
  connection_started = true;
  
  

  uint64_t msg_ackno = msg.ackno.value().unwrap(isn_, last_segment_recieved_);
  // check for impossible msg_ackno
  if (msg_ackno > last_segment_sent_) {
    return;
  }
  // RTO Timer: If all outstanding data has been acknowledged stop the retransmission timer
  if (last_segment_recieved_ <= msg_ackno && msg_ackno == last_segment_sent_) {
    last_ackno_received = msg_ackno;
    stop_rto_alarm();
    update_last_segment_recieved_and_clean(msg_ackno);
    return;
  }

  // RTO Timer: If we confirm receipt of new data
  if (last_segment_recieved_ < msg_ackno) {
    last_ackno_received = msg_ackno;
    update_last_segment_recieved_and_clean(msg_ackno);
    stop_rto_alarm();

    // If there is outstanding data leftover -> set_RTO_alarm + reset consec retransmissions
    if (last_segment_recieved_ <= last_segment_sent_) {
      set_rto_alarm();
    }
    // Else: all outstanding data has been acknowledged
  }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  /*
  If RTO Timer is expired:
      Retransmit the earlist segement not acknowledged
      If window_size is nonzero
        keep track of consec retransmissions and increment
        double value of curr_rto_
      Reset RTO Timer
  */  
  if (check_rto_alarm(ms_since_last_tick)) {
      TCPSenderMessage retransmit_msg = outstanding_segments[last_segment_recieved_];
      transmit(retransmit_msg);
      // update RTO and start new aalarm
      if (window_size != 0) {
        curr_RTO_ms_ *= 2;
      }
      set_rto_alarm();
  }
}

void TCPSender::set_rto_alarm() {
  rto_timer_is_on_ = true;
  rto_timer_ttl_ = curr_RTO_ms_;
}

bool TCPSender::check_rto_alarm( uint64_t ms_since_last_tick ) {
  // return false if timer is off
  if (!rto_timer_is_on_) {
    return false;
  }
  // check for expired timer
  if (ms_since_last_tick >= rto_timer_ttl_) {
    rto_timer_ttl_ = 0;
    rto_timer_is_on_ = false;
    return true;
  }
  // update tick + return false
  rto_timer_ttl_ -= ms_since_last_tick;
  return false;
}

void TCPSender::stop_rto_alarm() {
  rto_timer_is_on_ = false;
  curr_RTO_ms_ = initial_RTO_ms_;
}