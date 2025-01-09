#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ), data_(), newRead_() {}

// push one byte at a time
void Writer::push( string data )
{
  for ( const char& byte : data ) {
    // break if full
    if ( this->capacity_ == 0 ) {
      break;
    }
    // push one byte at a time, check capacity
    this->data_.emplace( byte );
    this->capacity_--;
    this->bytes_pushed_++;
    this->newRead_ += byte;
  }

  return;
}

void Writer::close()
{
  this->open_ = false;
}

bool Writer::is_closed() const
{
  return !this->open_;
}

uint64_t Writer::available_capacity() const
{
  return this->capacity_;
}

uint64_t Writer::bytes_pushed() const
{
  return this->bytes_pushed_;
}

string_view Reader::peek() const
{
  return string_view( this->newRead_ );
}

void Reader::pop( uint64_t len )
{

  uint64_t idx = len;
  while ( this->data_.size() != 0 && len != 0 ) {
    this->data_.pop();
    len--;
    this->bytes_popped_++;
    this->capacity_++;
  }

  // change this->newRead using substr, saves memory
  this->newRead_ = this->newRead_.substr( idx );
}

bool Reader::is_finished() const
{
  return this->data_.size() == 0 && !this->open_;
}

uint64_t Reader::bytes_buffered() const
{
  return this->bytes_pushed_ - this->bytes_popped_;
}

uint64_t Reader::bytes_popped() const
{
  return this->bytes_popped_;
}
