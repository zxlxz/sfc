#pragma once

#include "sfc/core/option.h"

namespace sfc::num {

template <trait::int_ T>
consteval auto max_value() -> T {
  if constexpr (trait::uint_<T>) {
    return static_cast<T>(~T{0});
  } else {
    return static_cast<T>(~(T{1} << (sizeof(T) * 8 - 1)));
  }
}

template <trait::int_ T>
consteval auto min_value() -> T {
  if constexpr (trait::uint_<T>) {
    return T{0};
  } else {
    return static_cast<T>(T{1} << (sizeof(T) * 8 - 1));
  }
}

template <class T>
constexpr auto abs(T val) -> T {
  return val >= 0 ? val : 0 - val;
}

template <class T>
constexpr auto min(T a, T b) -> T {
  return a < b ? a : b;
}

template <class T>
constexpr auto max(T a, T b) -> T {
  return a < b ? b : a;
}

template <trait::uint_ T>
constexpr auto next_power_of_two(T n, T init_val = 1U) -> T {
  auto x = init_val;
  while (x < n) {
    x = x << 1;
  }
  return x;
}

auto to_str(slice::Slice<char> buf, const void* val, char type = 0) noexcept -> str::Str;
auto to_str(slice::Slice<char> buf, trait::int_ auto val, char type = 0) noexcept -> str::Str;
auto to_str(slice::Slice<char> buf, trait::flt_ auto val, u32 prec = 6, char type = 0) noexcept -> str::Str;

template <trait::int_ T>
auto from_str(str::Str buf, u32 radix = 0) noexcept -> Option<T>;

template <trait::flt_ T>
auto from_str(str::Str buf) noexcept -> Option<T>;

auto flt_eq_ulp(f64 a, f64 b, u32 ulp = 4) noexcept -> bool;

}  // namespace sfc::num

namespace sfc::panicking {

void expect_flt_eq(auto a, auto b, u32 ulp = 4, Location loc = {}) noexcept {
  if (num::flt_eq_ulp(a, b, ulp)) {
    return;
  }
  panicking::panic_fmt(loc, "panicking::expect.flt(`{}`==`{}`) failed", a, b);
}

void expect_flt_ne(auto a, auto b, u32 ulp = 4, Location loc = {}) noexcept {
  if (!num::flt_eq_ulp(a, b, ulp)) {
    return;
  }
  panicking::panic_fmt(loc, "panicking::expect.flt(`{}`!=`{}`) failed", a, b);
}

}  // namespace sfc::panicking
