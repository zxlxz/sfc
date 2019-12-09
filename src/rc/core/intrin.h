#pragma once

#include "rc/core/mod.h"

namespace rc::intrin {

template <class T>
auto ctlz(const T& val) -> u32 {
  static_assert(rc::is_integeral<T>());
  if constexpr (sizeof(T) < sizeof(u64)) {
    return u32(::__builtin_clz(u32(val)));
  } else {
    return u32(::__builtin_clzll(u64(val)));
  }
}

template <class T>
auto cttz(const T& val) -> u32 {
  static_assert(rc::is_integeral<T>());

  if constexpr (sizeof(T) < sizeof(u64)) {
    return u32(::__builtin_ctz(u32(val)));
  } else {
    return u32(::__builtin_ctzll(u64(val)));
  }
}

}  // namespace rc::intrin
