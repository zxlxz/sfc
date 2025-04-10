#include "assert.h"

namespace sfc::test {

auto flt_eq_ulp(f64 a, f64 b, u32 ulp) -> bool {
  if (__builtin_isnan(a) || __builtin_isnan(b)) {
    return false;
  }

  if (__builtin_isinf(a) || __builtin_isinf(b)) {
    return a == b;
  }

  const auto ia = __builtin_bit_cast(i64, a);
  const auto ib = __builtin_bit_cast(i64, b);

  if ((ia ^ ib) < 0) {
    return a == b;
  }

  return __builtin_llabs(ia - ib) <= ulp;
}

}  // namespace sfc::test