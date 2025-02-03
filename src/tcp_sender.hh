#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"
#include <map>

#include <functional>

class TCPSender
{
public:
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( ByteStream&& input, Wrap32 isn, uint64_t initial_RTO_ms )
    : input_( std::move( input ) ), isn_( isn ), initial_RTO_ms_( initial_RTO_ms ), curr_RTO_ms_( initial_RTO_ms )
  {}

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage make_empty_message() const;

  /* Receive and process a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Type of the `transmit` function that the push and tick methods can use to send messages */
  using TransmitFunction = std::function<void( const TCPSenderMessage& )>;

  /* Push bytes from the outbound stream */
  void push( const TransmitFunction& transmit );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called */
  void tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit );



  // Accessors
  uint64_t sequence_numbers_in_flight() const;  // For testing: how many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // For testing: how many consecutive retransmissions have happened?
  const Writer& writer() const { return input_.writer(); }
  const Reader& reader() const { return input_.reader(); }
  Writer& writer() { return input_.writer(); }

private:
  Reader& reader() { return input_.reader(); }

    /* Set the Retransmission timer alarm, generally references RTO value */
  void set_rto_alarm();

  /* Check the existing Retransmission timer alarm given time since last tick information, returns true if timer has expired, false otherwise */
  bool check_rto_alarm( uint64_t ms_since_last_tick );

  /* Stop the RTO alarm if all outstanding data has been acknowledged */
  void stop_rto_alarm();

  /* Push Packet of given # of Bytes*/
  void push_packet(uint64_t size, TCPSenderMessage &cur_message, const TransmitFunction& transmit, bool false_window, bool add_fin_bit);

  // update the expected ackno to the first, segment to which we have not recieved a reply; also cleans map
  void update_last_segment_recieved_and_clean(uint64_t msg_ackno);

  ByteStream input_;
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;
  uint64_t curr_RTO_ms_;

  // state for tcp_sender
  uint64_t last_segment_recieved_ = 0;
  uint64_t last_ackno_received = 0;
  uint64_t last_segment_sent_ = 0;
  std::map<uint64_t, TCPSenderMessage> outstanding_segments = {};
  uint64_t bytes_outstanding = 0;

  // window size defaults to 0 to start a simple packet process to receive true window_size
  uint64_t window_size = 1;
  bool connection_started = false;
  bool sent_fin_bit = false;
  bool RST = false;
  

  // state(s) for retransimission timer; only used in rto_timer_functions
  bool rto_timer_is_on_ = false;
  uint64_t rto_timer_ttl_ = 0;
};
