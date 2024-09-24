#pragma once

#include "sfc/core.h"

namespace sfc::math {

static constexpr f64 PI = 3.14159265358979323846;

using num::ceil;
using num::fabs;
using num::floor;
using num::fmax;
using num::fmin;
using num::fmod;
using num::round;

[[sfc_inline]] inline auto exp(f32 x) -> f32 {
  return __builtin_expf(x);
}

[[sfc_inline]] inline auto exp(f64 x) -> f64 {
  return __builtin_exp(x);
}

[[sfc_inline]] inline auto log(f32 x) -> f32 {
  return __builtin_logf(x);
}

[[sfc_inline]] inline auto log(f64 x) -> f64 {
  return __builtin_log(x);
}

[[sfc_inline]] inline auto sqrt(f32 x) -> f32 {
  return __builtin_sqrtf(x);
}

[[sfc_inline]] inline auto sqrt(f64 x) -> f64 {
  return __builtin_sqrt(x);
}

[[sfc_inline]] inline auto pow(f32 x, f32 y) -> f32 {
  return __builtin_powf(x, y);
}

[[sfc_inline]] inline auto pow(f64 x, f64 y) -> f64 {
  return __builtin_pow(x, y);
}

[[sfc_inline]] inline auto cos(f32 x) -> f32 {
  return __builtin_cosf(x);
}

[[sfc_inline]] inline auto cos(f64 x) -> f64 {
  return __builtin_cos(x);
}

[[sfc_inline]] inline auto sin(f32 x) -> f32 {
  return __builtin_sinf(x);
}

[[sfc_inline]] inline auto sin(f64 x) -> f64 {
  return __builtin_sin(x);
}

[[sfc_inline]] inline auto tan(f32 x) -> f32 {
  return __builtin_tanf(x);
}

[[sfc_inline]] inline auto tan(f64 x) -> f64 {
  return __builtin_tan(x);
}

[[sfc_inline]] inline auto hypot(f32 x, f32 y) -> f32 {
  return __builtin_hypotf(x, y);
}

[[sfc_inline]] inline auto hypot(f64 x, f64 y) -> f64 {
  return __builtin_hypot(x, y);
}

[[sfc_inline]] inline auto asin(f32 y) -> f32 {
  return __builtin_asinf(y);
}

[[sfc_inline]] inline auto asin(f64 y) -> f64 {
  return __builtin_asin(y);
}

[[sfc_inline]] inline auto acos(f32 y) -> f32 {
  return __builtin_acosf(y);
}

[[sfc_inline]] inline auto acos(f64 y) -> f64 {
  return __builtin_acos(y);
}

[[sfc_inline]] inline auto atan2(f32 y, f32 x) -> f32 {
  return __builtin_atan2f(y, x);
}

[[sfc_inline]] inline auto atan2(f64 y, f64 x) -> f64 {
  return __builtin_atan2(y, x);
}

[[sfc_inline]] inline auto deg2rad(f32 degree) -> f32 {
  static constexpr f32 K = static_cast<f32>(PI / 180.0);
  return K * degree;
}

[[sfc_inline]] inline auto deg2rad(f64 degree) -> f64 {
  static constexpr f64 K = PI / 180.0;
  return K * degree;
}

[[sfc_inline]] inline auto rad2deg(f32 radian) -> f32 {
  static constexpr f32 K = static_cast<f32>(180.0 / PI);
  return K * radian;
}

[[sfc_inline]] inline auto rad2deg(f64 radian) -> f64 {
  static constexpr f64 K = 180.0 / PI;
  return K * radian;
}

}  // namespace sfc::math
