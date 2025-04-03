#pragma once

#include "sfc/math/funcs.h"

namespace sfc::math {

template <u32 N = 4096>
struct SinTbl {
  static_assert((N & (N - 1)) == 0);

  static constexpr auto kPI = static_cast<f32>(PI);
  static constexpr auto k2PI = static_cast<f32>(2 * PI);
  static constexpr auto kPI2 = static_cast<f32>(PI / 2);

  f32 _sin_tbl[N];
  f32 _asin_tbl[N];

 public:
  explicit SinTbl() {
    static constexpr auto n = static_cast<f32>(N);
    for (auto i = 0U; i < N; ++i) {
      const auto x = (static_cast<f32>(i) / n) * k2PI;  // [0..2pi)
      _sin_tbl[i] = __builtin_sinf(x);
    }
    for (auto i = 0U; i < N; ++i) {
      const auto x = (static_cast<f32>(i) / n) * 2 - 1;  // [-1, 1]
      _asin_tbl[i] = __builtin_asinf(x);
    }
  }

  static auto instance() -> const SinTbl& {
    static SinTbl res{};
    return res;
  }

  auto sin(f32 x) const -> f32 {
    const auto k = static_cast<u32>((10.00 + x / k2PI) * N);
    return _sin_tbl[k & (N - 1)];
  }

  auto cos(f32 x) const -> f32 {
    const auto k = static_cast<u32>((10.25 + x / k2PI) * N);
    return _sin_tbl[k & (N - 1)];
  }

  auto asin(f32 x) const -> f32 {
    static constexpr auto n2 = static_cast<f32>(N) / 2;

    if (x <= -1) return -kPI2;
    if (x >= +1) return +kPI2;
    const auto k = static_cast<u32>((x + 1.0) * n2);
    return _asin_tbl[k];
  }

  auto acos(f32 x) const -> f32 {
    static constexpr auto n2 = static_cast<f32>(N) / 2;

    if (x <= -1) return kPI;
    if (x >= +1) return 0;
    const auto k = static_cast<u32>((x + 1.0) * n2);
    return kPI2 - _asin_tbl[k];
  }
};

static inline auto fast_sin(f32 x) -> f32 {
  static auto tbl = SinTbl<>::instance();
  return tbl.sin(x);
}

static inline auto fast_cos(f32 x) -> f32 {
  static auto tbl = SinTbl<>::instance();
  return tbl.cos(x);
}

static inline auto fast_asin(f32 x) -> f32 {
  static auto tbl = SinTbl<>::instance();
  return tbl.asin(x);
}

static inline auto fast_acos(f32 x) -> f32 {
  static auto tbl = SinTbl<>::instance();
  return tbl.acos(x);
}

}  // namespace sfc::math
