#pragma once

#include "trait.h"

namespace sfc::mem {

struct inplace_t {};

template <class T>
union Uninit {
  T _0;

  [[sfc_inline]] Uninit() = default;
  [[sfc_inline]] ~Uninit() = default;
  [[sfc_inline]] Uninit(const Uninit&) = delete;

  [[sfc_inline]] auto operator&() -> T* {
    return &_0;
  }

  [[sfc_inline]] auto operator&() const -> const T* {
    return &_0;
  }

  [[sfc_inline]] auto operator->() -> T* {
    return &_0;
  }

  [[sfc_inline]] auto operator->() const -> const T* {
    return &_0;
  }
};

template <class T, class F>
[[sfc_inline]] inline auto bit_cast(F f) -> T {
  static_assert(sizeof(T) == sizeof(F));
  return __builtin_bit_cast(T, f);
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
