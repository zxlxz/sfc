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
  if constexpr (__is_trivially_copyable(T)) {
    return *src;
  } else {
    return static_cast<T&&>(*src);
  }
}

template <class T>
[[gnu::always_inline]] inline void write(T* dst, T val) noexcept {
  if constexpr (__is_trivially_copyable(T)) {
    *dst = val;
  } else {
    new (dst) T(static_cast<T&&>(val));
  }
}

template <class T>
[[gnu::always_inline]] inline void write_bytes(T* dst, u8 val, usize cnt) noexcept {
  if (cnt == 0) return;
  __builtin_memset(dst, val, cnt * sizeof(T));
}

template <class T>
[[gnu::always_inline]] inline void drop(T* ptr, usize cnt = 1) noexcept {
  for (auto i = 0UL; i < cnt; ++i) {
    ptr[i].~T();
  }
}

// all sfc object is memoveable, so we can just copy the bytes
template <class T>
[[gnu::always_inline]] inline void copy(const T* src, T* dst, usize cnt) noexcept {
  if (cnt == 0) return;
  __builtin_memmove(static_cast<void*>(dst), static_cast<const void*>(src), cnt * sizeof(T));
}

// all sfc object is memoveable, so we can just copy the bytes
template <class T>
[[gnu::always_inline]] inline void copy_nonoverlapping(const T* src, T* dst, usize cnt) noexcept {
  if (cnt == 0) return;
  __builtin_memcpy(static_cast<void*>(dst), static_cast<const void*>(src), cnt * sizeof(T));
}

template <class T>
[[gnu::always_inline]] inline void uninit_move(T* src, T* dst, usize cnt) noexcept {
  ptr::copy_nonoverlapping(src, dst, cnt);
}

template <class T>
[[gnu::always_inline]] inline void shift_elements_left(T* ptr, usize len, usize offset) noexcept {
  if (len == 0 || offset == 0) return;
  ptr::copy(ptr, ptr - offset, len);
}

template <class T>
[[gnu::always_inline]] inline void shift_elements_right(T* ptr, usize len, usize offset) noexcept {
  if (len == 0 || offset == 0) return;
  ptr::copy(ptr, ptr + offset, len);
}

}  // namespace sfc::ptr
