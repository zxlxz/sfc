#pragma once

#include "sfc/core/trait.h"

namespace sfc::slice {
template <class T>
struct Slice;
}

namespace sfc::str {
struct Str;
}

namespace sfc::num {

template <trait::int_ T>
constexpr auto max_value() -> T {
  if constexpr (trait::uint_<T>) {
    return static_cast<T>(~T{0});
  } else {
    return static_cast<T>(~(T{1} << (sizeof(T) * 8 - 1)));
  }
}

template <trait::int_ T>
constexpr auto min_value() -> T {
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

auto to_str(slice::Slice<char> buf, trait::int_ auto val, char type = 0) noexcept -> str::Str;
auto to_str(slice::Slice<char> buf, trait::flt_ auto val, u32 precision = 6, char type = 0) noexcept -> str::Str;

auto from_str(str::Str, trait::int_ auto& val) noexcept -> bool;
auto from_str(str::Str, trait::flt_ auto& val) noexcept -> bool;

auto flt_eq_ulp(f64 a, f64 b, u32 ulp = 4) noexcept -> bool;

}  // namespace sfc::num
