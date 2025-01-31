#include "tcp_sender.hh"
#include "debug.hh"
#include "tcp_config.hh"
#include "cmath"

using namespace std;

// This function is for testing only; don't add extra state to support it.
uint64_t TCPSender::sequence_numbers_in_flight() const
{
  debug( "unimplemented sequence_numbers_in_flight() called" );
  return {};
}

// This function is for testing only; don't add extra state to support it.
uint64_t TCPSender::consecutive_retransmissions() const
{
  return log2( curr_RTO_ms_ / initial_RTO_ms_);
}

void TCPSender::push_packet(uint64_t size, TCPSenderMessage &cur_message, const TransmitFunction& transmit, bool false_window) {
  string_view bytestream_state = input_.reader().peek();
  cur_message.payload = bytestream_state.substr(0, size);
  // seqno -> bytes popped from stream
  cur_message.seqno = Wrap32::wrap(reader().bytes_popped(), isn_);
  if (false_window) {
    input_.reader().pop(size);
  }
  cur_message.FIN = (input_.reader().is_finished() && input_.reader().bytes_buffered() == 0);
  transmit(cur_message);
}

void TCPSender::push( const TransmitFunction& transmit )
{
  uint64_t bytes_buffered_init = input_.reader().bytes_buffered();
  // return if input is empty or bytestream is closed
  if ( bytes_buffered_init == 0 ) {
    return;
  }

  TCPSenderMessage cur_message;
  size_t empty_message_size = cur_message.sequence_length();
  // if window_size is 0 and this is not the first message, always send a packet of size one to recieve new window size information
  if (window_size == 0 && connection_started) {
    push_packet(1, cur_message, transmit, true);
  }

  // otherwise, transmit as big of a message as you can
  size_t curr_space_in_connection = window_size - bytes_outstanding - empty_message_size; // 7 bytes of non-payload info
  size_t payload_size = min(TCPConfig::MAX_PAYLOAD_SIZE, curr_space_in_connection);

  // assuming window_size is bytes free in the bytestream; if less then 7 bytes if available cannot create a TCPMessage
  while (window_size - bytes_outstanding > empty_message_size) {
    curr_space_in_connection = window_size - bytes_outstanding - empty_message_size; // 7 bytes of non-payload info
    payload_size = min(TCPConfig::MAX_PAYLOAD_SIZE, curr_space_in_connection);
    push_packet(payload_size, cur_message, transmit, false);
    // add to dictionary - assuming passing by value here
    outstanding_segments[last_segment_sent_] = cur_message;
    bytes_outstanding += cur_message.sequence_length();
    last_segment_sent_ += cur_message.sequence_length();
  }
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  TCPSenderMessage cur_message;
  cur_message.seqno = Wrap32::wrap(reader().bytes_popped(), isn_);
  return cur_message;
}

/*
Goal of this function is to find the ackno of the first byte of the first, unacknowledged segment.
The first unackno. segment is always the key before, a key k s.t. k > msg_ackno.
*/
void TCPSender::update_expected_ackno_and_clean(uint64_t msg_ackno) {
  for (auto& pair : outstanding_segments) {
    uint64_t key = pair.first;
    if (key >= msg_ackno) {
      break;
    }
    // decrement bytes_outstanding can clean map
    bytes_outstanding -= outstanding_segments[expected_ackno_].sequence_length();
    outstanding_segments.erase(expected_ackno_);
    expected_ackno_ = key;
  }
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // if the msg has no value, skip it
  if (!msg.ackno.has_value()) {
    return;
  }

  // TODO: Clean collection of outstanding segments; add below to a helper function under RTO + decrement bytes_outstanding
  connection_started = true;
  window_size = msg.window_size;

  uint64_t msg_ackno = msg.ackno.value().unwrap(isn_, expected_ackno_);
  // RTO Timer: If all outstanding data has been acknowledged stop the retransmission timer
  if (expected_ackno_ <= msg_ackno && msg_ackno == last_segment_sent_) {
    stop_rto_alarm();
    update_expected_ackno_and_clean(msg_ackno);
    return;
  }

  // RTO Timer: If we confirm receipt of new data
  if (expected_ackno_ <= msg_ackno) {
    update_expected_ackno_and_clean(msg_ackno);
    stop_rto_alarm();

    // If there is outstanding data leftover -> set_RTO_alarm + reset consec retransmissions
    if (expected_ackno_ <= last_segment_sent_) {
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
      TCPSenderMessage retransmit_msg = outstanding_segments[expected_ackno_];
      transmit(retransmit_msg);
      // update RTO and start new aalarm
      curr_RTO_ms_ *= 2;
      set_rto_alarm();
  }
  
  
 
  

  debug( "unimplemented tick({}, ...) called", ms_since_last_tick );
  (void)transmit;
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