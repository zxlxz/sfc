#pragma once

#include "sfc/core/trait.h"

namespace sfc::num {

template <class T>
constexpr auto is_int() -> bool {
  return __is_same(T, char) ||                                         // char
         __is_same(T, signed char) || __is_same(T, unsigned char) ||   // byte
         __is_same(T, short) || __is_same(T, unsigned short) ||        // short
         __is_same(T, int) || __is_same(T, unsigned int) ||            // int
         __is_same(T, long) || __is_same(T, unsigned long) ||          // long
         __is_same(T, long long) || __is_same(T, unsigned long long);  // long long
}

template <class T>
constexpr auto is_flt() -> bool {
  return __is_same(T, float) || __is_same(T, double) || __is_same(T, long double);
}

template <class T>
constexpr auto is_signed() -> bool {
  return (is_int<T>() && (T{0} - T{1} < 0)) || is_flt<T>();
}

template <class T>
constexpr auto is_unsigned() -> bool {
  return is_int<T>() && (T{0} - T{1} > 0);
}

template <class T>
constexpr auto min_value() -> T {
  static_assert(is_int<T>());

  if constexpr (is_unsigned<T>()) {
    return 0;
  } else {
    return T(T(1) << T(8 * sizeof(T) - 1));
  }
}

template <class T>
constexpr auto max_value() -> T {
  static_assert(is_int<T>());

  if constexpr (num::is_unsigned<T>()) {
    return T(~T(0));
  } else {
    return T(~num::min_value<T>());
  }
}

template <class T>
constexpr auto abs(T val) -> T {
  static_assert(is_int<T>() || is_flt<T>());

  return val >= 0 ? val : 0 - val;
}

template <class T>
inline auto saturating_add(T lhs, T rhs) -> T {
  static_assert(is_int<T>());

  auto res = T{0};
  if (__builtin_add_overflow(lhs, rhs, &res)) {
    return num::max_value<T>();
  }
  return res;
}

template <class T>
inline auto saturating_sub(T lhs, T rhs) -> T {
  static_assert(is_int<T>());

  auto res = T{0};
  if (__builtin_sub_overflow(lhs, rhs, &res)) {
    return num::min_value<T>();
  }
  return res;
}

}  // namespace sfc::num
