#pragma once

#include "sfc/core/trait.h"

namespace sfc::mem {

template <class T>
inline void drop(T& x) {
  x.~T();
}

template <class T>
inline auto move(T& x) -> T&& {
  return static_cast<T&&>(x);
}

template <class T>
inline void swap(T& x, T& y) noexcept {
  auto z = static_cast<T&&>(x);
  x = static_cast<T&&>(y);
  y = static_cast<T&&>(z);
}

template <class T>
inline auto replace(T& dst, T val) -> T {
  auto res = static_cast<T&&>(dst);
  dst = static_cast<T&&>(val);
  return res;
}

template <class T>
inline auto take(T& dst) -> T {
  auto res = static_cast<T&&>(dst);
  dst = T{};
  return res;
}

}  // namespace sfc::mem

template <class T>
constexpr void* operator new(sfc::usize, T* ptr) noexcept {
  return ptr;
}
