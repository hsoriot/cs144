#include "wrapping_integers.hh"
#include <cstdint>

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  (void)n;
  (void)zero_point;
  return zero_point + static_cast<uint32_t>(n);
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  // (void)zero_point;
  // (void)checkpoint;
  
  uint64_t my_low32 {raw_value_ - zero_point.raw_value_};
  uint64_t check_low32 {checkpoint & mask_low32};
  uint64_t check_high32{checkpoint & mask_high32};
  uint64_t my {check_high32 | my_low32};
  if(my > Base and my_low32 > check_low32 and (my_low32 - check_low32) > (Base) / 2)
    return my - Base;
  if(my < mask_high32 and check_low32 > my_low32 and check_low32 - my_low32 > (Base / 2))
    return my + Base;
  return my;
}
