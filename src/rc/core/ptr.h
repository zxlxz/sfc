#pragma once

#include "rc/core/mem.h"

#ifndef __PLACEMENT_NEW_INLINE
#define __PLACEMENT_NEW_INLINE
inline auto operator new(rc::usize, void* addr) noexcept -> void* {
  return addr;
}
inline void operator delete(void*, void*)noexcept { return; }
#endif

namespace rc::ptr {

template <class T>
constexpr auto null() -> T* {
  return nullptr;
}

template <class T, class F>
constexpr auto cast(F* p) {
  if constexpr (rc::is_const<F>()) {
    return static_cast<const T*>(static_cast<const void*>(p));
  } else {
    return static_cast<T*>(static_cast<void*>(p));
  }
}

template <class T>
void drop_n(T* p) {
  mem::drop(*p);
}

template <class T>
void drop_n(T* p, usize n) {
  const auto end = p + n;
  for (; p != end; ++p) {
    mem::drop(*p);
  }
}

template <class T>
auto read(const T* p) noexcept -> T {
  return T{static_cast<T&&>(*const_cast<T*>(p))};
}

template <class T, class... U>
auto write(T* p, U&&... val) noexcept -> void {
  ::new (p) T(static_cast<U&&>(val)...);
}

template <class T, class... U>
auto replace(T* p, U&&... val) noexcept -> T {
  auto ret = ptr::read(p);
  ptr::write(p, static_cast<U&&>(val)...);
  return ret;
}

template <class T>
auto copy(const T* src, T* dst, usize cnt) -> void {
  __builtin_memcpy(dst, src, cnt * sizeof(T));
}

template <class T>
auto move(const T* src, T* dst, usize cnt) -> void {
  __builtin_memmove(dst, src, cnt * sizeof(T));
}

template <class T>
auto mshr(T* p, usize off, usize cnt) -> void {
  __builtin_memmove(p, p + off, cnt * sizeof(T));
}

template <class T>
auto mshl(T* p, usize off, usize cnt) -> void {
  __builtin_memmove(p, p - off, cnt * sizeof(T));
}

template <class T>
auto write_bytes(T* p, u8 val, usize cnt) -> void {
  __builtin_memset(p, val, cnt * sizeof(T));
}

}  // namespace rc::ptr
