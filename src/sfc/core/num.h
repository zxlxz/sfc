#pragma once

#include "sfc/core/mod.h"

namespace sfc::num {

template <class T>
constexpr auto abs(T val) -> T {
  return val >= 0 ? val : 0 - val;
}

template <class T>
constexpr auto(min)(T a, T b) -> T {
  return a < b ? a : b;
}

template <class T>
constexpr auto(max)(T a, T b) -> T {
  return a < b ? b : a;
}

auto flt_eq_ulp(f32 a, f32 b, u32 ulp = 4) -> bool;
auto flt_eq_ulp(f64 a, f64 b, u32 ulp = 4) -> bool;

}  // namespace sfc::num
