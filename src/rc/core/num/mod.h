#pragma once

#include "rc/core/intrin.h"

namespace rc::num {

template <class T>
constexpr auto max_value() -> T {
  static_assert(rc::is_integeral<T>());

  if constexpr (rc::is_unsigned<T>()) {
    return ~T(0);
  }
  if constexpr (rc::is_signed<T>()) {
    return ~T(1 << (8 * sizeof(T) - 1));
  }
}

template <class T>
constexpr auto abs(T val) -> T {
  return val < 0 ? T(-val) : val;
}

template <class T>
auto one_less_than_next_power_of_two(T val) -> T {
  static_assert(rc::is_unsigned<T>());
  if (val <= 1) {
    return 0;
  }
  const auto cnt = intrin::ctlz(val - 1);
  const auto ret = num::max_value<T>() >> cnt;
  return T(ret);
}

template <class T>
auto next_power_of_two(T val) -> T {
  static_assert(rc::is_unsigned<T>());
  return num::one_less_than_next_power_of_two(val) + 1;
}

}  // namespace rc::num
