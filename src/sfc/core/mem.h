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

}  // namespace sfc::mem

#if !defined(__PLACEMENT_NEW_INLINE) && !defined(_LIBCPP_NEW)
inline void* operator new(sfc::usize, void* ptr) noexcept {
  return ptr;
}
inline void operator delete(void*, void*) noexcept {
  return;
}
#endif

#if defined(_MSC_VER) && !defined(__clang__)
extern "C" void* __cdecl memset(void*, int, unsigned __int64);
extern "C" void* __cdecl memcpy(void*, const void*, unsigned __int64);
extern "C" void* __cdecl memmove(void*, const void*, unsigned __int64);
#pragma intrinsic(memset)
#pragma intrinsic(memcpy)
#pragma intrinsic(memmove)

#define __builtin_memset  memset
#define __builtin_memcpy  memcpy
#define __builtin_memmove memmove
#endif
