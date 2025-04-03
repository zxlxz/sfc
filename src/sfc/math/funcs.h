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

inline auto exp(f32 x) -> f32 {
  return __builtin_expf(x);
}

inline auto exp(f64 x) -> f64 {
  return __builtin_exp(x);
}

inline auto log(f32 x) -> f32 {
  return __builtin_logf(x);
}

inline auto log(f64 x) -> f64 {
  return __builtin_log(x);
}

inline auto sqrt(f32 x) -> f32 {
  return __builtin_sqrtf(x);
}

inline auto sqrt(f64 x) -> f64 {
  return __builtin_sqrt(x);
}

inline auto pow(f32 x, f32 y) -> f32 {
  return __builtin_powf(x, y);
}

inline auto pow(f64 x, f64 y) -> f64 {
  return __builtin_pow(x, y);
}

inline auto cos(f32 x) -> f32 {
  return __builtin_cosf(x);
}

inline auto cos(f64 x) -> f64 {
  return __builtin_cos(x);
}

inline auto sin(f32 x) -> f32 {
  return __builtin_sinf(x);
}

inline auto sin(f64 x) -> f64 {
  return __builtin_sin(x);
}

inline auto tan(f32 x) -> f32 {
  return __builtin_tanf(x);
}

inline auto tan(f64 x) -> f64 {
  return __builtin_tan(x);
}

inline auto hypot(f32 x, f32 y) -> f32 {
  return __builtin_hypotf(x, y);
}

inline auto hypot(f64 x, f64 y) -> f64 {
  return __builtin_hypot(x, y);
}

inline auto asin(f32 y) -> f32 {
  return __builtin_asinf(y);
}

inline auto asin(f64 y) -> f64 {
  return __builtin_asin(y);
}

inline auto acos(f32 y) -> f32 {
  return __builtin_acosf(y);
}

inline auto acos(f64 y) -> f64 {
  return __builtin_acos(y);
}

inline auto atan2(f32 y, f32 x) -> f32 {
  return __builtin_atan2f(y, x);
}

inline auto atan2(f64 y, f64 x) -> f64 {
  return __builtin_atan2(y, x);
}

inline auto deg2rad(f32 degree) -> f32 {
  static constexpr f32 K = static_cast<f32>(PI / 180.0);
  return K * degree;
}

inline auto deg2rad(f64 degree) -> f64 {
  static constexpr f64 K = PI / 180.0;
  return K * degree;
}

inline auto rad2deg(f32 radian) -> f32 {
  static constexpr f32 K = static_cast<f32>(180.0 / PI);
  return K * radian;
}

inline auto rad2deg(f64 radian) -> f64 {
  static constexpr f64 K = 180.0 / PI;
  return K * radian;
}

}  // namespace sfc::math
