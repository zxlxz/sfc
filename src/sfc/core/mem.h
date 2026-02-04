#pragma once

#include "sfc/core/trait.h"

namespace sfc::mem {

struct place_t {
  void* _ptr;
};

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
  if constexpr (requires { x.swap(y); }) {
    x.swap(y);
  } else {
    auto z = static_cast<T&&>(x);
    x = static_cast<T&&>(y);
    y = static_cast<T&&>(z);
  }
}

template <class T, class U>
inline void assign(T& dst, U&& src) noexcept {
  dst = static_cast<U&&>(src);
}

template <class T, class U>
inline auto replace(T& dst, U&& src) noexcept -> T {
  auto tmp = static_cast<T&&>(dst);
  dst = static_cast<U&&>(src);
  return tmp;
}

template <class T>
inline auto take(T& dst) noexcept -> T {
  auto res = static_cast<T&&>(dst);
  dst = T{};
  return res;
}

template <trait::tv_copy_ T>
inline auto as_bytes(const T& x) noexcept -> const u8 (&)[sizeof(T)] {
  return reinterpret_cast<const u8(&)[sizeof(T)]>(x);
}

template <trait::tv_copy_ T>
inline auto as_bytes_mut(T& x) noexcept -> u8 (&)[sizeof(T)] {
  return reinterpret_cast<u8(&)[sizeof(T)]>(x);
}

}  // namespace sfc::mem

constexpr void* operator new(sfc::usize, sfc::mem::place_t p) noexcept {
  return p._ptr;
}

constexpr void operator delete(void*, sfc::mem::place_t) noexcept {
  return;
}
