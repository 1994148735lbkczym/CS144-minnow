#include "tcp_receiver.hh"
#include "debug.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  // check for SYN flag and store ISN properly
  if (message.SYN && !this->received_isn_) {
    this->received_isn_ = true;
    this->zero_point_ = message.seqno;
    // update index to count ISN index
    this->first_unassembled_index_++;
  }

  // set error if RST flag and return
  if (message.RST) {
    reassembler_.reader().set_error();
    this->received_rst_ = true;
    return;
  }

  // cannot write if no isn; return
  if (!this->received_isn_) {
    return;
  }

  // calculate first_index by unwrapping message; update if this is the message where we received syn
  uint64_t first_index = message.seqno.unwrap(this->zero_point_, this->first_unassembled_index_);
  if (message.SYN && message.payload.size() != 0) {
    first_index++;
  }
  // insert message to reassembler, and update state; -1 since SYN does not count for reassembler
  uint64_t reassembler_capacity_pre = reassembler_.writer().available_capacity();
  reassembler_.insert(first_index - 1, message.payload, message.FIN);
  uint64_t reassembler_capacity_post = reassembler_.writer().available_capacity();
  this->first_unassembled_index_ += reassembler_capacity_pre - reassembler_capacity_post;
  
  // add FIN byte index if needed
  if (reassembler_.writer().is_closed()) {
    this->first_unassembled_index_++;
  }

  
}

TCPReceiverMessage TCPReceiver::send() const
{
  TCPReceiverMessage message;
  // Add optional ackno if we have recieved an ISN
  if (this->received_isn_) {
    message.ackno = Wrap32::wrap(this->first_unassembled_index_, this->zero_point_);
  }
  // WINDOWSIZE = MAX_SIZE - USED_INDEXES(not including syn/fin)
  message.window_size = min((uint64_t)UINT16_MAX, reassembler_.writer().available_capacity());

  // RST is true if bytestream detected error or Receiver had an error packet
  message.RST = this->reassembler_.writer().has_error() || this->received_rst_;

  return message;
}
