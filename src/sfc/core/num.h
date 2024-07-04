#pragma once

#include "sfc/core/mod.h"

namespace sfc::slice {
template <class T>
struct Slice;
}

namespace sfc::str {
struct Str;
}

namespace sfc::num {

template <class T>
constexpr auto abs(T val) -> T {
  return val >= 0 ? val : 0 - val;
}

auto flt_eq_ulp(f32 a, f32 b, u32 ulp = 4) -> bool;
auto flt_eq_ulp(f64 a, f64 b, u32 ulp = 4) -> bool;

auto int2str(slice::Slice<char> buf, auto val, char type = 0) -> str::Str;
auto flt2str(slice::Slice<char> buf, auto val, u32 prec = 6, char type = 0) -> str::Str;

}  // namespace sfc::num
