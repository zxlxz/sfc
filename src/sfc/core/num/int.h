#pragma once

#include "sfc/core/trait.h"

namespace sfc::num {

template <trait::UInt T>
constexpr auto min_value() -> T {
  return T{0};
}

template <trait::SInt T>
constexpr auto min_value() -> T {
  return T(T(1) << T(8 * sizeof(T) - 1));
}

template <trait::UInt T>
constexpr auto max_value() -> T {
  return T(~0);
}

template <trait::SInt T>
constexpr auto max_value() -> T {
  return T(~num::min_value<T>());
}

template <class T>
constexpr auto abs(T val) -> T {
  return val >= 0 ? val : 0 - val;
}

template <trait::Int T>
[[sfc_inline]] inline auto saturating_add(T lhs, T rhs) -> T {
  auto res = T{0};
  if (__builtin_add_overflow(lhs, rhs, &res)) {
    return num::max_value<T>();
  }
  return res;
}

template <trait::Int T>
[[sfc_inline]] inline auto saturating_sub(T lhs, T rhs) -> T {
  auto res = T{0};
  if (__builtin_sub_overflow(lhs, rhs, &res)) {
    return T{0};
  }
  return res;
}

}  // namespace sfc::num
