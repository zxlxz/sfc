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

template <class T>
concept UInt = __is_same(T, unsigned char) || __is_same(T, unsigned short) ||
               __is_same(T, unsigned int) || __is_same(T, unsigned long) ||
               __is_same(T, unsigned long long);

template <class T>
concept SInt = __is_same(T, signed char) || __is_same(T, signed short) ||
               __is_same(T, signed int) || __is_same(T, signed long) ||
               __is_same(T, signed long long);

template <class T>
concept Int = SInt<T> || UInt<T>;

template <class T>
concept Float = __is_same(T, float) || __is_same(T, double) || __is_same(T, long double);

auto flt_eq_ulp(f32 a, f32 b, u32 ulp = 4) -> bool;
auto flt_eq_ulp(f64 a, f64 b, u32 ulp = 4) -> bool;

auto int2str(slice::Slice<char> buf, auto val, char type = 0) -> str::Str;
auto flt2str(slice::Slice<char> buf, auto val, u32 prec = 6, char type = 0) -> str::Str;

}  // namespace sfc::num
