#pragma once

#include "sfc/core/trait.h"

namespace sfc::num {

template <trait::int_ T>
static constexpr auto max_value() -> T {
  static constexpr auto SHIFT = sizeof(T) * 8 - 1;
  if constexpr (trait::uint_<T>) {
    return static_cast<T>(~T{0});
  } else {
    return static_cast<T>(~(T{1} << SHIFT));
  }
}

template <trait::int_ T>
static constexpr auto min_value() -> T {
  static constexpr auto SHIFT = sizeof(T) * 8 - 1;
  if constexpr (trait::uint_<T>) {
    return 0;
  } else {
    return static_cast<T>(T{1} << SHIFT);
  }
}

template <trait::int_ T>
constexpr auto abs(T val) -> T {
  return val >= 0 ? val : 0 - val;
}

template <trait::uint_ T>
constexpr auto ctlz(T val) -> u32 {
  static constexpr auto EXT_CNT = (sizeof(u64) - sizeof(T)) * 8;
  if (val == 0) return sizeof(T) * 8;
  return static_cast<u32>(__builtin_clzll(val) - EXT_CNT);
}

template <trait::uint_ T>
constexpr auto next_power_of_two(T n) -> T {
  if ((n & (n - 1)) == 0) {
    return n;
  } else {
    const auto shift = sizeof(T) * 8 - num::ctlz(n);
    return static_cast<T>(T{1} << shift);
  }
}

template <trait::uint_ T>
constexpr auto saturating_sub(T a, T b) -> T {
  return a < b ? 0 : a - b;
}

auto flt_eq_ulp(f64 a, f64 b, u32 ulp = 4) noexcept -> bool;

}  // namespace sfc::num
