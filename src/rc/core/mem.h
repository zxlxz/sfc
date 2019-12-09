#pragma once

#include "rc/core/intrin.h"

namespace rc::mem {

template <class T>
auto drop(T& val) -> void {
  if constexpr (!__is_trivially_destructible(T)) return;
  val.~T();
}

template <class T>
auto swap(T& x, T& y) -> void {
  T z(static_cast<T&&>(x));
  ::new (&x) T(static_cast<T&&>(y));
  ::new (&y) T(static_cast<T&&>(z));
}

}  // namespace rc::mem
