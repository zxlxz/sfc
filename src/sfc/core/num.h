#pragma once

#include "sfc/core/mod.h"

namespace sfc::num {

template <class T>
concept sint_ = same_<T, signed char> || same_<T, short> || same_<T, int> || same_<T, long> || same_<T, long long>;

template <class T>
concept uint_ = same_<T, unsigned char> || same_<T, unsigned short> || same_<T, unsigned int> ||
                same_<T, unsigned long> || same_<T, unsigned long long>;

template <class T>
concept int_ = sint_<T> || uint_<T>;

template <class T>
concept float_ = same_<T, float> || same_<T, double>;

template <int_ T>
static constexpr auto max_value() -> T {
  if constexpr (uint_<T>) {
    return static_cast<T>(~T{0});
  } else {
    return static_cast<T>(~(T{1} << (sizeof(T) * 8 - 1)));
  }
}

template <sint_ T>
static constexpr auto min_value() -> T {
  return static_cast<T>(T{1} << (sizeof(T) * 8 - 1));
}

template <int_ T>
constexpr auto abs(T val) -> T {
  return val >= 0 ? val : 0 - val;
}

template <uint_ T>
constexpr auto next_power_of_two(T n) -> T {
  if ((n & (n - 1)) == 0) {
    return n;
  }
  auto res = T{1U};
  while (res < n) {
    res <<= 1;
  }
  return res;
}

template <uint_ T>
constexpr auto saturating_sub(T a, T b) -> T {
  return a < b ? 0 : a - b;
}

auto flt_eq_ulp(f64 a, f64 b, u32 ulp = 4) noexcept -> bool;

}  // namespace sfc::num
