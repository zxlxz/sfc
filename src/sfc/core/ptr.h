#pragma once

#include "sfc/core/mem.h"
#include "sfc/core/expect.h"

namespace sfc::ptr {

template <class T>
struct Unique {
  T* _ptr = nullptr;

 public:
  Unique(T* ptr = nullptr) noexcept : _ptr{ptr} {}

  Unique(Unique&& other) noexcept : _ptr{other._ptr} {
    other._ptr = nullptr;
  }

  Unique& operator=(Unique&& other) noexcept {
    _ptr = mem::take(other._ptr);
    return *this;
  }

  auto ptr() const noexcept -> T* {
    return _ptr;
  }

  explicit operator bool() const noexcept {
    return _ptr != nullptr;
  }

  auto operator->() const -> const T* {
    return _ptr;
  }

  auto operator->() -> T* {
    return _ptr;
  }

  auto operator*() const -> const T& {
    sfc::expect(_ptr != nullptr, "ptr::Unique::*: deref null");
    return *_ptr;
  }

  auto operator*() -> T& {
    sfc::expect(_ptr != nullptr, "ptr::Unique::*: deref null");
    return *_ptr;
  }

 public:
  // trait: Clone
  auto clone() const noexcept -> Unique {
    return Unique{_ptr};
  }
};

template <class T>
inline auto read(T* src) noexcept -> T {
  if constexpr (trait::tv_copy_<T>) {
    return *src;
  } else {
    auto val = static_cast<T&&>(*src);
    src->~T();
    return val;
  }
}

template <class T>
inline void write(T* dst, auto&& val) noexcept {
  if constexpr (__is_trivially_copyable(T)) {
    *dst = static_cast<decltype(val)&&>(val);
  } else {
    new (dst) mem::MaybeUninit<T>{static_cast<decltype(val)&&>(val)};
  }
}

template <class T>
inline void write_bytes(T* dst, u8 val, usize cnt) noexcept {
  if (cnt == 0) {
    return;
  }

  __builtin_memset(dst, val, cnt * sizeof(T));
}

template <class T>
inline void drop_in_place([[maybe_unused]] T* ptr, [[maybe_unused]] usize cnt) noexcept {
  if constexpr (!trait::tv_dtor_<T>) {
    for (auto end = ptr + cnt; ptr != end; ++ptr) {
      ptr->~T();
    }
  }
}

template <class T>
inline void copy(const T* src, T* dst, usize cnt) noexcept {
  if (cnt == 0) {
    return;
  }

  if constexpr (__is_trivially_copyable(T)) {
    __builtin_memmove(dst, src, cnt * sizeof(T));
  } else {
    if (dst < src) {
      for (auto ps = src, pd = dst; ps != src + cnt; ++ps, ++pd) {
        *pd = *ps;
      }
    } else if (dst > src) {
      for (auto ps = src + cnt - 1, pd = dst + cnt - 1; ps != src - 1; --ps, --pd) {
        *pd = *ps;
      }
    }
  }
}

template <class T>
inline void copy_nonoverlapping(const T* src, T* dst, usize cnt) noexcept {
  if (cnt == 0) {
    return;
  }

  if constexpr (__is_trivially_copyable(T)) {
    __builtin_memcpy(dst, src, cnt * sizeof(T));
  } else {
    for (auto ps = src, pd = dst; ps != src + cnt; ++ps, ++pd) {
      *pd = *ps;
    }
  }
}

template <class T>
inline void uninit_copy(const T* src, T* dst, usize cnt) noexcept {
  if (cnt == 0) {
    return;
  }

  if constexpr (__is_trivially_copyable(T)) {
    __builtin_memcpy(dst, src, cnt * sizeof(T));
  } else {
    for (auto ps = src, pd = dst; ps != src + cnt; ++ps, ++pd) {
      new (pd) mem::MaybeUninit<T>{*ps};
    }
  }
}

template <class T>
inline void uninit_move(T* src, T* dst, usize cnt) noexcept {
  if (cnt == 0) {
    return;
  }

  if constexpr (__is_trivially_copyable(T)) {
    __builtin_memcpy(dst, src, sizeof(T));
  } else {
    for (auto ps = src, pd = dst; ps != src + cnt; ++ps, ++pd) {
      new (pd) mem::MaybeUninit<T>{static_cast<T&&>(*ps)};
      ps->~T();
    }
  }
}

template <class T>
inline void shift_elements_left(T* ptr, usize len, usize offset) noexcept {
  if (len == 0) {
    return;
  }

  if constexpr (__is_trivially_copyable(T)) {
    __builtin_memmove(ptr - offset, ptr, len * sizeof(T));
  } else {
    for (auto p = ptr; p != ptr + len; ++p) {
      new (p + offset) mem::MaybeUninit<T>{static_cast<T&&>(*p)};
      p->~T();
    }
  }
}

template <class T>
inline void shift_elements_right(T* ptr, usize len, usize offset) noexcept {
  if (len == 0) {
    return;
  }

  if constexpr (__is_trivially_copyable(T)) {
    __builtin_memmove(ptr + offset, ptr, len * sizeof(T));
  } else {
    for (auto p = ptr + len - 1; p != ptr - 1; --p) {
      new (p - offset) mem::MaybeUninit<T>{static_cast<T&&>(*p)};
      p->~T();
    }
  }
}

}  // namespace sfc::ptr
