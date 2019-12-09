#pragma once

#include "rc/core/intrin.h"

namespace rc::mem {

template <class T>
void drop(T& val) {
  if constexpr (!__is_trivially_destructible(T)) {
    val.~T();
  }
}

template <class T>
void swap(T& x, T& y) {
  T z(static_cast<T&&>(x));
  ::new (&x) T(static_cast<T&&>(y));
  ::new (&y) T(static_cast<T&&>(z));
}

}  // namespace rc::mem
