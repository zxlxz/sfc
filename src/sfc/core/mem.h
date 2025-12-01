#pragma once

#include "sfc/core/mod.h"

namespace sfc::mem {

template <class T>
inline void drop(T& x) {
  x.~T();
}

template <class T>
inline auto move(T& x) noexcept -> T&& {
  return static_cast<T&&>(x);
}

template <class T>
inline void swap(T& x, T& y) noexcept {
  auto z = static_cast<T&&>(x);
  x = static_cast<T&&>(y);
  y = static_cast<T&&>(z);
}

template <class T>
inline auto replace(T& dst, T val) noexcept -> T {
  auto res = static_cast<T&&>(dst);
  dst = static_cast<T&&>(val);
  return res;
}

template <class T>
inline auto take(T& dst) noexcept -> T {
  auto res = static_cast<T&&>(dst);
  dst = T{};
  return res;
}

template <class T>
inline auto as_bytes(const T& x) noexcept -> const u8 (&)[sizeof(T)] {
  static_assert(__is_trivially_copyable(T));
  return reinterpret_cast<const u8(&)[sizeof(T)]>(x);
}

template <class T>
inline auto as_bytes_mut(T& x) noexcept -> u8 (&)[sizeof(T)] {
  static_assert(__is_trivially_copyable(T));
  return reinterpret_cast<u8(&)[sizeof(T)]>(x);
}

}  // namespace sfc::mem
