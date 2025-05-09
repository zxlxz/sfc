#pragma once

#include "sfc/core/trait.h"

namespace sfc::mem {

struct inplace_t {};

template <class T>
struct Ref {
  T* _ptr;

 public:
  Ref(T& val) noexcept : _ptr{&val} {}

  void operator=(T& val) {
    _ptr = &val;
  }

  operator T&() {
    return *_ptr;
  }
};

template <class T, class F>
inline auto bit_cast(F f) -> T {
  static_assert(sizeof(T) == sizeof(F));
  if constexpr (__is_same(T, F)) {
    return f;
  } else {
    return __builtin_bit_cast(T, f);
  }
}

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
  x      = static_cast<T&&>(y);
  y      = static_cast<T&&>(z);
}

template <class T>
inline auto replace(T& dst, T val) -> T {
  auto res = static_cast<T&&>(dst);
  dst      = static_cast<T&&>(val);
  return res;
}

template <class T>
inline auto take(T& dst) -> T {
  auto res = static_cast<T&&>(dst);
  dst      = T{};
  return res;
}

}  // namespace sfc::mem

template <class T>
constexpr auto operator new(sfc::usize size, T* ptr) noexcept -> void* {
  static_assert(sizeof(T) != 0);
  return ptr;
}
