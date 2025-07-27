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

constexpr inline auto nan() -> f32 {
  return __builtin_nanf("");
}

constexpr inline auto inf() -> f32 {
  return __builtin_inff();
}

constexpr inline auto isnan(auto x) -> bool {
  return __builtin_isnan(x);
}

constexpr inline auto isinf(auto x) -> bool {
  return __builtin_isinf(x);
}

constexpr inline auto fabs(f32 x) -> f32 {
  return __builtin_fabsf(x);
}

constexpr inline auto fabs(f64 x) -> f64 {
  return __builtin_fabs(x);
}

constexpr inline auto round(f32 x) -> f32 {
  return __builtin_roundf(x);
}

constexpr inline auto round(f64 x) -> f64 {
  return __builtin_round(x);
}

constexpr inline auto floor(f32 x) -> f32 {
  return __builtin_floorf(x);
}

constexpr inline auto floor(f64 x) -> f64 {
  return __builtin_floor(x);
}

constexpr inline auto ceil(f32 x) -> f32 {
  return __builtin_ceilf(x);
}

constexpr inline auto ceil(f64 x) -> f64 {
  return __builtin_ceil(x);
}

constexpr inline auto trunc(f32 x) -> f32 {
  return __builtin_truncf(x);
}

constexpr inline auto trunc(f64 x) -> f64 {
  return __builtin_trunc(x);
}

constexpr inline auto fmod(f32 x, f32 y) -> f32 {
  return __builtin_fmodf(x, y);
}

constexpr inline auto fmod(f64 x, f64 y) -> f64 {
  return __builtin_fmod(x, y);
}

constexpr inline auto fmin(f32 x, f32 y) -> f32 {
  return __builtin_fminf(x, y);
}

constexpr inline auto fmin(f64 x, f64 y) -> f64 {
  return __builtin_fmin(x, y);
}

constexpr inline auto fmax(f32 x, f32 y) -> f32 {
  return __builtin_fmaxf(x, y);
}

constexpr inline auto fmax(f64 x, f64 y) -> f64 {
  return __builtin_fmax(x, y);
}

auto flt_eq_ulp(f32 a, f32 b, u32 ulp = 4) -> bool;
auto flt_eq_ulp(f64 a, f64 b, u32 ulp = 4) -> bool;

auto flt2str(slice::Slice<char> buf, auto val, u32 prec = 6, char type = 0) -> str::Str;

}  // namespace sfc::num
