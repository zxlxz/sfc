#pragma once

#include "sfc/core/trait.h"

template <class T>
inline void* operator new([[maybe_unused]] sfc::usize size, T* p) noexcept {
  return p;
}

namespace sfc::mem {

struct Layout {
  usize size{0};
  usize align{1};

 public:
  template <class T>
  static auto of() noexcept -> Layout {
    return {.size = sizeof(T), .align = alignof(T)};
  }

  template <class T>
  static auto for_value(const T&) noexcept -> Layout {
    return {.size = sizeof(T), .align = alignof(T)};
  }

  template <class T>
  static auto array(usize n) noexcept -> Layout {
    return {.size = n * sizeof(T), .align = alignof(T)};
  }
};

template <class T>
inline auto move(T& x) noexcept -> T&& {
  return static_cast<T&&>(x);
}

template <class T>
inline void drop(T& x) {
  x.~T();
}

template <class T>
inline void swap(T& x, T& y) noexcept {
  if constexpr (requires { x.swap(y); }) {
    x.swap(y);
  } else {
    auto z = mem::move(x);
    x = mem::move(y);
    y = mem::move(z);
  }
}

template <class T>
inline void forget(T& x) noexcept {
  alignas(T) char tmp[sizeof(T)];
  new (tmp) T{mem::move(x)};
}

template <class T>
inline void assign(T& dst, T src) noexcept {
  dst = mem::move(src);
}

template <class T>
inline auto replace(T& dst, T src) noexcept -> T {
  auto tmp = mem::move(dst);
  dst = mem::move(src);
  return tmp;
}

template <class T>
inline auto take(T& dst) noexcept -> T {
  auto res = mem::move(dst);
  dst = T{};
  return res;
}

template <class T>
inline auto as_bytes(const T& x) noexcept -> const u8 (&)[sizeof(T)] {
  static_assert(__is_trivially_copyable(T));
  return reinterpret_cast<const u8(&)[sizeof(T)]>(x);
}

template <class T>
inline auto as_mut_bytes(T& x) noexcept -> u8 (&)[sizeof(T)] {
  static_assert(__is_trivially_copyable(T));
  return reinterpret_cast<u8(&)[sizeof(T)]>(x);
}

}  // namespace sfc::mem
