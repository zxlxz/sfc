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
constexpr __forceinline auto null()->T* {
  return nullptr;
}

template <class T, class F>
constexpr __forceinline auto cast(F* p) {
  if constexpr (rc::is_const<F>()) {
    return static_cast<const T*>(static_cast<const void*>(p));
  } else {
    return static_cast<T*>(static_cast<void*>(p));
  }
}

template <class T>
__forceinline auto drop(T* ptr, usize cnt)->void {
  auto end = ptr + cnt;
  for (auto p = ptr; p != end; ++p) {
    mem::drop(*p);
  }
}

template <class T>
__forceinline auto read(const T* p) noexcept -> T {
  return T{static_cast<T&&>(*const_cast<T*>(p))};
}

template <class T, class ...U>
__forceinline auto write(T* p, U&&... val) noexcept -> void {
  ::new (p) T(static_cast<U&&>(val)...);
}

template <class T, class ...U>
__forceinline auto replace(T* p, U&&... val) noexcept -> T {
  auto ret = ptr::read(p);
  ptr::write(p, static_cast<U&&>(val)...);
  return ret;
}

template <class T>
__forceinline auto _copy(const T* src, T* dst, usize size) -> void {
  __builtin_memcpy(dst, src, size);
}

template <class T>
__forceinline auto _move(const T* src, T* dst, usize size) -> void {
  __builtin_memmove(dst, src, size);
}

template <class T>
__forceinline auto copy(const T* src, T* dst, usize cnt) -> void {
  __builtin_memcpy(dst, src, cnt * sizeof(T));
}

template <class T>
__forceinline auto move(const T* src, T* dst, usize cnt) -> void {
  __builtin_memmove(dst, src, cnt * sizeof(T));
}

template <class T>
__forceinline auto mshr(T* p, usize off, usize cnt) -> void {
  __builtin_memmove(p, p + off, cnt * sizeof(T));
}

template <class T>
__forceinline auto mshl(T* p, usize off, usize cnt) -> void {
  __builtin_memmove(p, p - off, cnt * sizeof(T));
}

template <class T>
__forceinline auto write_bytes(T* p, u8 val, usize cnt) -> void {
  __builtin_memset(p, val, cnt * sizeof(T));
}

}  // namespace rc::ptr
