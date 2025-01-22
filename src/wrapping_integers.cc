#include "wrapping_integers.hh"
#include "debug.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // ISN + zero_point % 2^32
  return Wrap32{static_cast<uint32_t>(zero_point.raw_value_ + (n & ((1ULL << 32) - 1)))};;
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // wrap the checkpoint
  Wrap32 c_point = Wrap32::wrap(checkpoint, zero_point);
  // abs value logic
  uint32_t diff = (c_point.raw_value_ - this->raw_value_ < this->raw_value_ - c_point.raw_value_)
                ? (c_point.raw_value_ - this->raw_value_)
                : (this->raw_value_ - c_point.raw_value_);
  
  uint64_t tmp = checkpoint + diff;
  uint64_t tmp_sub = (checkpoint < diff) ? ((uint32_t)checkpoint - diff) : checkpoint - diff;

  if (Wrap32::wrap(tmp, zero_point) == *this) 
    return tmp;
  if (Wrap32::wrap(tmp_sub, zero_point) == *this) 
    return tmp_sub;
  return 0;
}
