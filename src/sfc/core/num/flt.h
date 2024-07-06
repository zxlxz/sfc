#pragma once

#include "sfc/core/mem.h"

namespace sfc::num {

[[sfc_inline]] constexpr inline auto nan() -> f32 {
  return __builtin_nanf("");
}

[[sfc_inline]] constexpr inline auto inf() -> f32 {
  return __builtin_inff();
}

[[sfc_inline]] inline auto isnan(auto x) -> bool {
  return __builtin_isnan(x);
}

[[sfc_inline]] inline auto isinf(auto x) -> bool {
  return __builtin_isinf(x);
}

[[sfc_inline]] inline auto fabs(f32 x) -> f32 {
  return __builtin_fabsf(x);
}

[[sfc_inline]] inline auto fabs(f64 x) -> f64 {
  return __builtin_fabs(x);
}

[[sfc_inline]] inline auto round(f32 x) -> f32 {
  return __builtin_roundf(x);
}

[[sfc_inline]] inline auto round(f64 x) -> f64 {
  return __builtin_round(x);
}

[[sfc_inline]] inline auto floor(f32 x) -> f32 {
  return __builtin_floorf(x);
}

[[sfc_inline]] inline auto floor(f64 x) -> f64 {
  return __builtin_floor(x);
}

[[sfc_inline]] inline auto ceil(f32 x) -> f32 {
  return __builtin_ceilf(x);
}

[[sfc_inline]] inline auto ceil(f64 x) -> f64 {
  return __builtin_ceil(x);
}

[[sfc_inline]] inline auto trunc(f32 x) -> f32 {
  return __builtin_truncf(x);
}

[[sfc_inline]] inline auto trunc(f64 x) -> f64 {
  return __builtin_trunc(x);
}

[[sfc_inline]] inline auto fmod(f32 x, f32 y) -> f32 {
  return __builtin_fmodf(x, y);
}

[[sfc_inline]] inline auto fmod(f64 x, f64 y) -> f64 {
  return __builtin_fmod(x, y);
}

[[sfc_inline]] inline auto fmin(f32 x, f32 y) -> f32 {
  return __builtin_fminf(x, y);
}

[[sfc_inline]] inline auto fmin(f64 x, f64 y) -> f64 {
  return __builtin_fmin(x, y);
}

[[sfc_inline]] inline auto fmax(f32 x, f32 y) -> f32 {
  return __builtin_fmaxf(x, y);
}

[[sfc_inline]] inline auto fmax(f64 x, f64 y) -> f64 {
  return __builtin_fmax(x, y);
}

inline auto flt_eq(f64 a, f64 b, u32 err = 4) -> bool {
  if (__builtin_isnan(a) || __builtin_isnan(b)) {
    return false;
  }
  if (a == b) {
    return true;
  }
  if (a < 0) {
    a = __builtin_fabs(a);
    b = __builtin_fabs(b);
  }
  auto ua = mem::bit_cast<u64>(a);
  auto ub = mem::bit_cast<u64>(b);
  const auto uc = ua < ub ? ub - ua : ua - ub;
  return uc < err;
}

}  // namespace sfc::num
