#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ), data_(), newRead_() {}

// push one byte at a time
void Writer::push( string data )
{
  uint64_t datasize = 0;
  for (const char &byte : data) {
    // break if full
    if (this->capacity_ == 0) {
      break;
    }
    // push one byte at a time, check capacity
    this->data_.emplace(byte);
    this->capacity_--;
    this->bytes_pushed_++;
    datasize++;
    this->newRead_ += byte;
  }
  
  return;
}

void Writer::close()
{
  // Your code here.
}

bool Writer::is_closed() const
{
  return {}; // Your code here.
}

uint64_t Writer::available_capacity() const
{
  return {}; // Your code here.
}

uint64_t Writer::bytes_pushed() const
{
  return {}; // Your code here.
}

string_view Reader::peek() const
{
  return {}; // Your code here.
}

void Reader::pop( uint64_t len )
{
  (void)len; // Your code here.
}

bool Reader::is_finished() const
{
  return {}; // Your code here.
}

uint64_t Reader::bytes_buffered() const
{
  return {}; // Your code here.
}

uint64_t Reader::bytes_popped() const
{
  return {}; // Your code here.
}

