#pragma once

#include "sfc/core/trait.h"

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
    auto z = static_cast<T&&>(x);
    x = static_cast<T&&>(y);
    y = static_cast<T&&>(z);
  }
}

template <class T>
inline void assign(T& dst, T src) noexcept {
  dst = static_cast<T&&>(src);
}

template <class T>
inline auto replace(T& dst, T src) noexcept -> T {
  auto tmp = static_cast<T&&>(dst);
  dst = static_cast<T&&>(src);
  return tmp;
}

template <class T>
inline auto take(T& dst) noexcept -> T {
  auto res = static_cast<T&&>(dst);
  dst = T{};
  return res;
}

template <trait::tv_copy_ T>
auto as_bytes(const T& self) -> const u8 (&)[sizeof(T)] {
  return *(const u8(*)[sizeof(T)])(&self);
}

template <trait::tv_copy_ T>
auto as_bytes_mut(T& self) -> u8 (&)[sizeof(T)] {
  return *(u8(*)[sizeof(T)])(&self);
}

}  // namespace sfc::mem

namespace sfc {
using mem::Layout;
}
