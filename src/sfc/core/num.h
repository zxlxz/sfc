#pragma once

#include "sfc/core/trait.h"

namespace sfc::num {

template <trait::int_ T>
consteval auto max_value() -> T {
  if constexpr (trait::uint_<T>) {
    return static_cast<T>(~T{0});
  } else {
    return static_cast<T>(~(T{1} << (sizeof(T) * 8 - 1)));
  }
}

template <trait::sint_ T>
consteval auto min_value() -> T {
  return static_cast<T>(T{1} << (sizeof(T) * 8 - 1));
}

template <trait::uint_ T>
constexpr auto saturating_sub(T a, T b) -> T {
  return a < b ? 0U : a - b;
}

template <trait::uint_ T>
constexpr auto align_up(T val, T align) -> T {
  const auto mask = T{align - 1};
  return (val + mask) & ~mask;
}

template <trait::uint_ T>
constexpr auto is_power_of_two(T val) -> bool {
  return (val & (val - 1)) == 0;
}

template <trait::uint_ T>
constexpr auto next_power_of_two(T n) -> T {
  auto t = T{1};
  while (t < n)
    t <<= 1;
  return t;
}

template <trait::int_ T>
constexpr auto uabs(T x) {
  static_assert(sizeof(T) <= sizeof(i64), "uabs: type is too large");

  const auto f = []<typename U>(U u, auto s) {
    if (u << 1 == 0) return u;  // handle min value
    return static_cast<U>(s >= 0 ? u : 0 - u);
  };

  if constexpr (sizeof(T) == sizeof(i8)) {
    return f(static_cast<u8>(x), x);
  } else if constexpr (sizeof(T) == sizeof(i16)) {
    return f(static_cast<u16>(x), x);
  } else if constexpr (sizeof(T) == sizeof(i32)) {
    return f(static_cast<u32>(x), x);
  } else if constexpr (sizeof(T) == sizeof(i64)) {
    return f(static_cast<u64>(x), x);
  } else {
    return x;
  }
}

auto flt_eq_ulp(f64 a, f64 b, u32 ulp = 4) noexcept -> bool;

}  // namespace sfc::num
