#include "sfc/core/num.h"
#include "sfc/core/str.h"

namespace sfc::num {

auto flt_eq_ulp(f64 a, f64 b, u32 ulp) noexcept -> bool {
  const auto ia = __builtin_bit_cast(i64, a);
  const auto ib = __builtin_bit_cast(i64, b);
  if ((ia ^ ib) < 0) {
    return a == b;
  }

  const auto diff = num::unsigned_abs(ia - ib);
  return diff <= ulp;
}

}  // namespace sfc::num
