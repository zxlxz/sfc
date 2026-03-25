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

template <class T>
constexpr auto abs(T val) -> T {
  return val >= 0 ? val : 0 - val;
}

template <trait::uint_ T>
constexpr auto saturating_sub(T a, T b) -> T {
  return a < b ? 0 : a - b;
}

template <trait::int_ T>
constexpr auto next_power_of_two(T n) -> T {
  auto x = T{1};
  while (x < n) {
    x *= 2;
  }
  return x;
}

template <trait::uint_ T>
constexpr auto align_up(T n, u64 align) -> T {
  const auto mask = align - 1;
  return static_cast<T>((n + mask) & ~mask);
}

auto flt_eq_ulp(f64 a, f64 b, u32 ulp = 4) noexcept -> bool;

}  // namespace sfc::num
