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
constexpr inline auto nan() -> T {
  if constexpr (__is_same(T, f32)) {
    return __builtin_nanf("");
  } else {
    return __builtin_nn("");
  }
}

template <class T>
constexpr inline auto inf() -> T {
  if constexpr (__is_same(T, f32)) {
    return __builtin_inff();
  } else {
    return __builtin_inf();
  }
}

constexpr inline auto isnan(auto x) -> bool {
  return __builtin_isnan(x);
}

constexpr inline auto isinf(auto x) -> bool {
  return __builtin_isinf(x);
}

template <class T>
constexpr inline auto fabs(T x) -> T {
  if constexpr (__is_same(T, f32)) {
    return __builtin_fabsf(x);
  } else {
    return __builtin_fabs(x);
  }
}

template <class T>
constexpr inline auto round(T x) -> T {
  if constexpr (__is_same(T, f32)) {
    return __builtin_roundf(x);
  } else {
    return __builtin_round(x);
  }
}

template <class T>
constexpr inline auto floor(T x) -> T {
  if constexpr (__is_same(T, f32)) {
    return __builtin_floorf(x);
  } else {
    return __builtin_floor(x);
  }
}

template <class T>
constexpr inline auto ceil(T x) -> T {
  if constexpr (__is_same(T, f32)) {
    return __builtin_ceilf(x);
  } else {
    return __builtin_ceil(x);
  }
}

template <class T>
constexpr inline auto trunc(T x) -> T {
  if constexpr (__is_same(T, f32)) {
    return __builtin_truncf(x);
  } else {
    return __builtin_trunc(x);
  }
}

template <class T>
constexpr inline auto fmod(T x, T y) -> T {
  if constexpr (__is_same(T, f32)) {
    return __builtin_fmodf(x, y);
  } else {
    return __builtin_fmod(x, y);
  }
}

template <class T>
constexpr inline auto fmin(T x, T y) -> T {
  if constexpr (__is_same(T, f32)) {
    return __builtin_fminf(x, y);
  } else {
    return __builtin_fmin(x, y);
  }
}

template <class T>
constexpr inline auto fmax(T x, T y) -> T {
  if constexpr (__is_same(T, f32)) {
    return __builtin_fmaxf(x, y);
  } else {
    return __builtin_fmax(x, y);
  }
}

auto flt_eq_ulp(f32 a, f32 b, u32 ulp = 4) -> bool;
auto flt_eq_ulp(f64 a, f64 b, u32 ulp = 4) -> bool;
auto int2str(slice::Slice<char> buf, auto val, char type = 0) -> str::Str;
auto flt2str(slice::Slice<char> buf, auto val, u32 prec = 6, char type = 0) -> str::Str;

}  // namespace sfc::num
