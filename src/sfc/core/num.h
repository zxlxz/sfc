#pragma once

#include "sfc/core/trait.h"

namespace sfc::num {

template <class T>
struct Int {
  T _val;

 public:
  static auto from_str(str::Str buf, u32 radix = 0) noexcept -> option::Option<T>;
  auto to_str(slice::Slice<char> buf, char type = 0) const noexcept -> str::Str;
};

template <class T>
struct Flt {
  T _val;

 public:
  static auto from_str(str::Str buf) noexcept -> option::Option<T>;
  auto to_str(slice::Slice<char> buf, u32 prec = 6, char type = 0) const noexcept -> str::Str;
};

template <class T>
constexpr auto abs(T val) -> T {
  return val >= 0 ? val : 0 - val;
}

template <trait::uint_ T>
constexpr auto next_power_of_two(T n, T init_val = 1U) -> T {
  auto x = init_val;
  while (x < n) {
    x *= 2;
  }
  return x;
}

auto flt_eq_ulp(f64 a, f64 b, u32 ulp = 4) noexcept -> bool;

}  // namespace sfc::num
