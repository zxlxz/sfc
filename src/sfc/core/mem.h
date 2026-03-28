#pragma once

#include "sfc/core/mod.h"

namespace sfc::mem {

template <class T>
[[gnu::always_inline]] inline void forget(T& x) noexcept {
  alignas(T) char tmp[sizeof(T)];
  new (tmp) T{static_cast<T&&>(x)};
}

template <class T>
[[gnu::always_inline]] inline void drop(T& x) {
  x.~T();
}

template <class T>
[[gnu::always_inline]] inline auto move(T& x) noexcept -> T&& {
  return static_cast<T&&>(x);
}

template <class T>
[[gnu::always_inline]] inline void swap(T& x, T& y) noexcept {
  if constexpr (requires { x.swap(y); }) {
    x.swap(y);
  } else {
    auto z = static_cast<T&&>(x);
    x = static_cast<T&&>(y);
    y = static_cast<T&&>(z);
  }
}

template <class T, class U>
[[gnu::always_inline]] inline void assign(T& dst, U&& src) noexcept {
  dst = static_cast<U&&>(src);
}

template <class T, class U>
[[gnu::always_inline]] inline auto replace(T& dst, U&& src) noexcept -> T {
  auto tmp = static_cast<T&&>(dst);
  dst = static_cast<U&&>(src);
  return tmp;
}

template <class T>
[[gnu::always_inline]] inline auto take(T& dst) noexcept -> T {
  auto res = static_cast<T&&>(dst);
  dst = T{};
  return res;
}

template <class T>
[[gnu::always_inline]] inline auto as_bytes(const T& x) noexcept -> const u8 (&)[sizeof(T)] {
  static_assert(__is_trivially_copyable(T));
  return reinterpret_cast<const u8(&)[sizeof(T)]>(x);
}

template <class T>
[[gnu::always_inline]] inline auto as_mut_bytes(T& x) noexcept -> u8 (&)[sizeof(T)] {
  static_assert(__is_trivially_copyable(T));
  return reinterpret_cast<u8(&)[sizeof(T)]>(x);
}

}  // namespace sfc::mem

template <class T>
[[gnu::always_inline]] inline void* operator new([[maybe_unused]] sfc::usize size, T* p) noexcept {
  return p;
}
