#pragma once

#include "sfc/core/mem.h"

namespace sfc::ptr {

template <class T>
struct Unique {
  T* _ptr = nullptr;

 public:
  [[gnu::always_inline]] Unique(T* ptr = nullptr) noexcept : _ptr{ptr} {}

  [[gnu::always_inline]] Unique(Unique&& other) noexcept : _ptr{other._ptr} {
    other._ptr = nullptr;
  }

  [[gnu::always_inline]] Unique& operator=(Unique&& other) noexcept {
    if (this != &other) {
      _ptr = other._ptr;
      other._ptr = nullptr;
    }
    return *this;
  }

  [[gnu::always_inline]] auto ptr() const noexcept -> T* {
    return _ptr;
  }

  [[gnu::always_inline]] auto operator->() const -> const T* {
    return _ptr;
  }

  [[gnu::always_inline]] auto operator->() -> T* {
    return _ptr;
  }

  [[gnu::always_inline]] auto operator*() const -> const T& {
    return *_ptr;
  }

  [[gnu::always_inline]] auto operator*() -> T& {
    return *_ptr;
  }

 public:
  // trait: Clone
  [[gnu::always_inline]] auto clone() const noexcept -> Unique {
    return Unique{_ptr};
  }
};

template <class T>
[[gnu::always_inline]] inline auto read(T* src) noexcept -> T {
  if constexpr (trait::tv_copy_<T>) {
    return *src;
  } else {
    auto val = static_cast<T&&>(*src);
    src->~T();
    return val;
  }
}

template <class T>
[[gnu::always_inline]] inline void write(T* dst, auto&& val) noexcept {
  if constexpr (__is_trivially_copyable(T)) {
    *dst = static_cast<decltype(val)&&>(val);
  } else {
    new (dst) T{static_cast<decltype(val)&&>(val)};
  }
}

template <class T>
[[gnu::always_inline]] inline void write_bytes(T* dst, u8 val, usize cnt) noexcept {
  if (cnt != 0) {
    __builtin_memset(dst, val, cnt * sizeof(T));
  }
}

template <class T>
[[gnu::always_inline]] inline void drop_in_place([[maybe_unused]] T* ptr, [[maybe_unused]] usize cnt) noexcept {
  if constexpr (!trait::tv_dtor_<T>) {
    for (auto end = ptr + cnt; ptr != end; ++ptr) {
      ptr->~T();
    }
  }
}

template <class T>
[[gnu::always_inline]] inline void copy(const T* src, T* dst, usize cnt) noexcept {
  if constexpr (__is_trivially_copyable(T)) {
    if (cnt != 0) {
      __builtin_memmove(dst, src, cnt * sizeof(T));
    }
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
[[gnu::always_inline]] inline void copy_nonoverlapping(const T* src, T* dst, usize cnt) noexcept {
  if constexpr (__is_trivially_copyable(T)) {
    if (cnt != 0) {
      __builtin_memcpy(dst, src, cnt * sizeof(T));
    }
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
      ptr::write(pd, *ps);
    }
  }
}

template <class T>
inline void uninit_move(T* src, T* dst, usize cnt) noexcept {
  if (cnt == 0) {
    return;
  }

  if constexpr (__is_trivially_copyable(T)) {
    __builtin_memcpy(dst, src, cnt * sizeof(T));
  } else {
    for (auto ps = src, pd = dst; ps != src + cnt; ++ps, ++pd) {
      ptr::write(pd, static_cast<T&&>(*ps));
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
      ptr::write(p + offset, static_cast<T&&>(*p));
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
      ptr::write(p - offset, static_cast<T&&>(*p));
      p->~T();
    }
  }
}

}  // namespace sfc::ptr
