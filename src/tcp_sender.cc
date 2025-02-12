#include "tcp_sender.hh"
#include "cmath"
#include "debug.hh"
#include "tcp_config.hh"

using namespace std;

// This function is for testing only; don't add extra state to support it.
uint64_t TCPSender::sequence_numbers_in_flight() const
{
  return last_segment_sent_ - last_ackno_received;
}

// This function is for testing only; don't add extra state to support it.
uint64_t TCPSender::consecutive_retransmissions() const
{
  return log2( curr_RTO_ms_ / initial_RTO_ms_ );
}

void TCPSender::push_packet( uint64_t size,
                             TCPSenderMessage& cur_message,
                             const TransmitFunction& transmit,
                             bool rst,
                             bool can_add_fin_bit )
{
  // Adding SYN Flag is first priority; update size after
  cur_message.SYN = last_segment_sent_ == 0;
  if ( cur_message.SYN && size != 0 ) {
    size--;
  }

  // Compute Seqno
  cur_message.seqno = Wrap32::wrap( last_segment_sent_, isn_ );

  // Add Fin Flag
  if ( can_add_fin_bit ) {
    cur_message.FIN = true;
    sent_fin_bit = true;
    size = min( size, size - 1 );
  }
  // Add Payload; Update input_ Byte Stream
  cur_message.payload = input_.reader().peek().substr( 0, size );
  input_.reader().pop( size );

  // Add RST Flag + Transmit
  cur_message.RST = rst;
  transmit( cur_message );

  // Update State + Timer
  outstanding_segments[last_segment_sent_] = cur_message;
  last_segment_sent_ += cur_message.sequence_length();
  bytes_outstanding += cur_message.sequence_length();
  if ( !rto_timer_is_on_ ) {
    set_rto_alarm();
  }
}

void TCPSender::push( const TransmitFunction& transmit )
{
  TCPSenderMessage cur_message;

  // Edge Case: If RST Flag has been raised transmit RST Packet
  if ( RST || input_.has_error() ) {
    push_packet( 0, cur_message, transmit, RST || input_.has_error(), false );
    return;
  }

  // Edge Case: Send just the SYN Flag only when Sender reaches out to Peer first
  if ( input_.reader().bytes_buffered() == 0 && !connection_started ) {
    push_packet( 0, cur_message, transmit, false, input_.writer().is_closed() );
    return;
  }

  // Edge Case: If Window Size is 0 send 1 byte packet to recieve new window
  if ( window_size == 0 && connection_started ) {
    if ( !bytes_outstanding ) { // Check for outstanding packets (ie. if Sender sent a 1 Byte Packet previously
                                // already)
      push_packet( 1, cur_message, transmit, false, input_.reader().is_finished() );
    }
    return;
  }

  // Compute Constant to construct largest TCP Message Possible
  size_t curr_space_in_connection = window_size - bytes_outstanding;
  // Payload Size is the min between the MAX_PAYLOAD_SIZE, Current Space in the Connection, and the Number of Bytes
  // Buffered in input_
  size_t payload_size
    = min( min( TCPConfig::MAX_PAYLOAD_SIZE, curr_space_in_connection ), input_.reader().bytes_buffered() );

  // Determine if SYN/FIN flags should be raised
  bool can_add_syn_bit = last_segment_sent_ == 0;
  bool can_add_fin_bit
    = ( input_.writer().is_closed() && payload_size >= input_.reader().bytes_buffered() && !sent_fin_bit );

  // Check if there is space in connection for SYN/FIN
  can_add_syn_bit = can_add_syn_bit && curr_space_in_connection - payload_size;
  can_add_fin_bit = can_add_fin_bit && curr_space_in_connection - payload_size;

  // While there is space in the connection && we have bytes we need to send
  while ( curr_space_in_connection > 0 && payload_size + ( can_add_fin_bit && !sent_fin_bit ) > 0 ) {
    // Re-evaluate the variables that could change after a packet is pushed (an iteration of the while loop passes)
    can_add_fin_bit
      = ( input_.writer().is_closed() && payload_size >= input_.reader().bytes_buffered() && !sent_fin_bit );
    can_add_fin_bit = ( can_add_fin_bit && curr_space_in_connection - payload_size );
    // Final Payload Size = Payload Size + Additional Flag Bits
    size_t overall_size = payload_size + can_add_fin_bit + can_add_syn_bit;
    push_packet( overall_size, cur_message, transmit, false, can_add_fin_bit );
    // Re-evaluate the variables that could change after a packet is pushed (an iteration of the while loop passes)
    curr_space_in_connection = window_size - bytes_outstanding;
    payload_size
      = min( min( TCPConfig::MAX_PAYLOAD_SIZE, curr_space_in_connection ), input_.reader().bytes_buffered() );
    can_add_fin_bit = ( input_.writer().is_closed() && input_.reader().bytes_buffered() < curr_space_in_connection
                        && !sent_fin_bit );
  }
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  TCPSenderMessage cur_message;
  cur_message.seqno = Wrap32::wrap( last_segment_sent_, isn_ );
  cur_message.RST = RST || input_.has_error();
  return cur_message;
}

/*
Goal of this function is to find the ackno of the first byte of the first, unacknowledged segment.
The first unackno. segment is always the key before, a key k s.t. k > msg_ackno.
*/
void TCPSender::update_last_segment_recieved_and_clean( uint64_t msg_ackno )
{
  // Loop through outstanding_segments and remove stored segments that have been acknowledged
  auto it = outstanding_segments.begin();
  while ( it != outstanding_segments.end() ) {
    // If this key is not acknowledged by msg_ackno, break
    if ( it->first >= msg_ackno ) {
      break;
    }
    // If: Segment has been completely acknowledged by the peer
    if ( it->first + it->second.sequence_length() <= msg_ackno ) {
      bytes_outstanding = last_segment_sent_ - msg_ackno;
      last_segment_recieved_ += it->second.sequence_length();
      it = outstanding_segments.erase( it ); // erase returns next valid iterator
    } else {
      // Else: Update Outstanding Bytes for a partial segment recognition
      bytes_outstanding = last_segment_sent_ - msg_ackno;
      ++it;
    }
  }
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Check RST Flag + Update Window
  RST = msg.RST || input_.has_error();
  window_size = msg.window_size;
  if ( RST ) {
    input_.set_error();
  }
  // If the message has no other value, skip it
  if ( !msg.ackno.has_value() ) {
    return;
  }
  // Update State
  connection_started = true;

  uint64_t msg_ackno = msg.ackno.value().unwrap( isn_, last_segment_recieved_ );
  // Check for an impossible ackno (greater then what we have sent)
  if ( msg_ackno > last_segment_sent_ ) {
    return;
  }
  // RTO Timer: If all outstanding data has been acknowledged stop the retransmission timer
  if ( last_segment_recieved_ <= msg_ackno && msg_ackno == last_segment_sent_ ) {
    last_ackno_received = msg_ackno;
    stop_rto_alarm();
    update_last_segment_recieved_and_clean( msg_ackno );
    return;
  }
  // RTO Timer: If we confirm receipt of new data, update state + alarm
  if ( last_segment_recieved_ < msg_ackno ) {
    last_ackno_received = msg_ackno;
    update_last_segment_recieved_and_clean( msg_ackno );
    stop_rto_alarm();
    // If there is outstanding data leftover -> set_RTO_alarm() + reset consecutive retransmission count
    if ( last_segment_recieved_ <= last_segment_sent_ ) {
      set_rto_alarm();
    }
  }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  // Check RTO Timer
  if ( check_rto_alarm( ms_since_last_tick ) ) {
    // Retransmit Segment
    TCPSenderMessage retransmit_msg = outstanding_segments[last_segment_recieved_];
    transmit( retransmit_msg );
    // Update Consecutive RTO Count
    if ( window_size != 0 ) {
      curr_RTO_ms_ *= 2;
    }
    set_rto_alarm();
  }
}

void TCPSender::set_rto_alarm()
{
  rto_timer_is_on_ = true;
  rto_timer_ttl_ = curr_RTO_ms_;
}

bool TCPSender::check_rto_alarm( uint64_t ms_since_last_tick )
{
  // Return False if timer is off
  if ( !rto_timer_is_on_ ) {
    return false;
  }
  // Check for expired timer
  if ( ms_since_last_tick >= rto_timer_ttl_ ) {
    rto_timer_ttl_ = 0;
    rto_timer_is_on_ = false;
    return true;
  }
  // Update tick + return false
  rto_timer_ttl_ -= ms_since_last_tick;
  return false;
}

void TCPSender::stop_rto_alarm()
{
  rto_timer_is_on_ = false;
  curr_RTO_ms_ = initial_RTO_ms_;
}