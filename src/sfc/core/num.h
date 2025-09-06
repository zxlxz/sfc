#pragma once

#include "sfc/core/mod.h"

namespace sfc::num {

template <class T>
constexpr auto abs(T val) -> T {
  return val >= 0 ? val : 0 - val;
}

auto flt_eq_ulp(f32 a, f32 b, u32 ulp = 4) -> bool;
auto flt_eq_ulp(f64 a, f64 b, u32 ulp = 4) -> bool;

}  // namespace sfc::num
