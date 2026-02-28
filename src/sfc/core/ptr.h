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
  static_assert(__is_trivially_copyable(T));
  __builtin_memset(dst, val, cnt * sizeof(T));
}

template <class T>
inline void drop_in_place([[maybe_unused]] T* ptr, [[maybe_unused]] usize cnt) noexcept {
  if constexpr (!__is_trivially_destructible(T)) {
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
      for (auto end = src + cnt; src != end; ++src, ++dst) {
        *dst = *src;
      }
    } else if (dst > src) {
      src += cnt - 1;
      dst += cnt - 1;
      for (auto end = src - cnt; src != end; --src, --dst) {
        *dst = *src;
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
    for (auto end = src + cnt; src != end; ++src, ++dst) {
      *dst = *src;
    }
  }
}

template <class S, class D>
inline void uninit_copy(S* src, D* dst, usize cnt) noexcept {
  static_assert(sizeof(S) == sizeof(D));
  if (cnt == 0) {
    return;
  }

  if constexpr (__is_trivially_copyable(S)) {
    __builtin_memcpy(dst, src, sizeof(S) * cnt);
  } else {
    for (auto end = src + cnt; src != end; ++src, ++dst) {
      ptr::write(dst, static_cast<D&&>(*src));
    }
  }
}

template <class T>
inline void shift_elements_left(T* src, usize len, usize offset) noexcept {
  if (len == 0 || offset == 0) {
    return;
  }

  const auto dst = src - offset;
  if constexpr (__is_trivially_copyable(T)) {
    __builtin_memmove(dst, src, len * sizeof(T));
  } else {
    for (auto idx = 0UL; idx < offset; ++idx) {
      ptr::write(dst + idx, static_cast<T&&>(src[idx]));
    }
    for (auto idx = offset; idx < len; ++idx) {
      dst[idx] = static_cast<T&&>(src[idx]);
    }
    for (auto idx = len - offset; idx < len; ++idx) {
      src[idx].~T();
    }
  }
}

template <class T>
inline void shift_elements_right(T* src, usize len, usize offset) noexcept {
  if (len == 0 || offset == 0) {
    return;
  }

  const auto dst = src + offset;
  if constexpr (__is_trivially_copyable(T)) {
    __builtin_memmove(dst, src, len * sizeof(T));
  } else {
    for (auto idx = len; idx > len - offset; --idx) {
      ptr::write(dst + idx - 1, static_cast<T&&>(src[idx - 1]));
    }
    for (auto idx = len - offset; idx > 0; --idx) {
      dst[idx - 1] = static_cast<T&&>(src[idx - 1]);
    }
    for (auto idx = offset; idx > 0; --idx) {
      src[idx].~T();
    }
  }
}

}  // namespace sfc::ptr
