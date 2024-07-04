#pragma once

#include "sfc/core.h"

namespace sfc::test {

inline auto flt_eq(f64 a, f64 b) -> bool {
  if (num::isnan(a) || num::isnan(b)) {
    return false;
  }
  if (a == b) {
    return true;
  }
  if (a < 0) {
    a = num::fabs(a);
    b = num::fabs(b);
  }
  auto ua = mem::bit_cast<u64>(a);
  auto ub = mem::bit_cast<u64>(b);
  const auto uc = ua < ub ? ub - ua : ua - ub;
  return uc < 4U;
}

}  // namespace sfc::test
