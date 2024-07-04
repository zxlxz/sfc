#pragma once

#include "trait.h"

namespace sfc::mem {

struct inplace_t {};

template <class T, class F>
[[sfc_inline]] inline auto bit_cast(F f) -> T {
  static_assert(sizeof(T) == sizeof(F));
#if __cplusplus >= 202002L
  return __builtin_bit_cast(T, f);
#else
  T res;
  __builtin_memcpy(&res, &f, sizeof(T));
  return res;
#endif
}

template <class T>
[[sfc_inline]] inline void drop(T& x) {
  x.~T();
}

template <class T>
[[sfc_inline]] inline auto move(T& x) -> T&& {
  return static_cast<T&&>(x);
}

template <class T>
[[sfc_inline]] inline void swap(T& x, T& y) {
  auto z = static_cast<T&&>(x);
  x = static_cast<T&&>(y);
  y = static_cast<T&&>(z);
}

template <class T>
[[sfc_inline]] inline auto replace(T& dst, T val) -> T {
  auto res = static_cast<T&&>(dst);
  dst = static_cast<T&&>(val);
  return res;
}

template <class T>
[[sfc_inline]] inline auto take(T& dst) -> T {
  auto res = static_cast<T&&>(dst);
  dst = T{};
  return res;
}

}  // namespace sfc::mem

[[sfc_inline]] inline void* operator new(sfc::usize, sfc::mem::inplace_t, void* p) {
  return p;
}
