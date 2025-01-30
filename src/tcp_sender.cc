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

void TCPSender::push( const TransmitFunction& transmit )
{
  debug( "unimplemented push() called" );
  (void)transmit;
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  debug( "unimplemented make_empty_message() called" );
  return {};
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // if the msg has no value, skip it
  if (!msg.ackno.has_value()) {
    return;
  }
  uint64_t msg_ackno = msg.ackno.value().unwrap(isn_, expected_ackno_);
  // RTO Timer: If all outstanding data has been acknowledged stop the retransmission timer
  if (expected_ackno_ <= msg_ackno && msg_ackno == last_segment_sent_) {
    stop_rto_alarm();
    expected_ackno_++;
    return;
  }

  // RTO Timer: If we confirm receipt of new data
  if (expected_ackno_ <= msg_ackno) {
    expected_ackno_ = msg_ackno + 1;
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