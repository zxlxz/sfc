#pragma once

#include "sfc/core/mem.h"

namespace sfc::ptr {

using nullptr_t = decltype(nullptr);

template <class T>
struct Unique {
  T* _ptr = nullptr;

 public:
  Unique() noexcept = default;

  Unique(nullptr_t) noexcept {}

  explicit Unique(T* ptr) noexcept : _ptr{ptr} {}

  ~Unique() noexcept {}

  Unique(Unique&& other) noexcept : _ptr{other._ptr} {
    other._ptr = nullptr;
  }

  Unique& operator=(Unique&& other) noexcept {
    _ptr = other._ptr;
    other._ptr = nullptr;
    return *this;
  }

  explicit operator bool() const noexcept {
    return _ptr != nullptr;
  }

  auto ptr() const noexcept -> T* {
    return _ptr;
  }

  auto operator->() const -> const T* {
    panicking::assert(_ptr != nullptr, "ptr::Unique::->: deref null");
    return _ptr;
  }

  auto operator->() -> T* {
    panicking::assert(_ptr != nullptr, "ptr::Unique::->: deref null");
    return _ptr;
  }

  auto operator*() const -> const T& {
    panicking::assert(_ptr != nullptr, "ptr::Unique::*: deref null");
    return *_ptr;
  }

  auto operator*() -> T& {
    panicking::assert(_ptr != nullptr, "ptr::Unique::*: deref null");
    return *_ptr;
  }
};

template <class T>
inline auto read(T* src) noexcept -> T {
  if constexpr (__is_trivially_copyable(T)) {
    return *src;
  } else {
    auto val = static_cast<T&&>(*src);
    src->~T();
    return val;
  }
}

template <class T, class... U>
inline void write(T* dst, U&&... args) noexcept {
  new (dst) T{static_cast<U&&>(args)...};
}

template <class T>
inline void write_bytes(T* dst, u8 val, usize cnt) noexcept {
  if constexpr (__is_trivially_copyable(T)) {
#if defined(_MSC_VER) && !defined(__clang__)
    memset(dst, val, cnt * sizeof(T));
#else
    __builtin_memset(dst, val, cnt * sizeof(T));
#endif
  } else {
    for (auto i = 0UL; i < cnt; ++i) {
      new (&dst[i]) T{static_cast<T&&>(val)};
    }
  }
}

template <class T>
inline void drop_in_place([[maybe_unused]] T* ptr, [[maybe_unused]] usize cnt) {
  if constexpr (!__is_trivially_destructible(T)) {
    for (auto i = 0UL; i < cnt; ++i) {
      ptr[i].~T();
    }
  }
}

template <class T>
inline void copy(const T* src, T* dst, usize cnt) {
  if constexpr (__is_trivially_copyable(T)) {
    cnt ? (void)__builtin_memmove(dst, src, cnt * sizeof(T)) : (void)0;
  } else {
    if (dst < src) {
      for (auto idx = 0UL; idx < cnt; ++idx) {
        dst[idx] = src[idx];
      }
    } else {
      for (const auto end = src; cnt > 0; --cnt) {
        dst[cnt - 1] = src[cnt - 1];
      }
    }
  }
}

template <class T>
inline void copy_nonoverlapping(const T* src, T* dst, usize cnt) {
  if constexpr (__is_trivially_copyable(T)) {
    cnt ? (void)__builtin_memcpy(dst, src, cnt * sizeof(T)) : (void)0;

  } else {
    for (auto idx = 0UL; idx < cnt; ++idx) {
      dst[idx] = src[idx];
    }
  }
}

template <class T>
inline void uninit_copy(const T* src, T* dst, usize cnt) {
  if (cnt == 0 || !src || !dst) {
    return;
  }
  if constexpr (__is_trivially_copyable(T)) {
    cnt ? (void)__builtin_memcpy(dst, src, sizeof(T) * cnt) : (void)0;
  } else {
    for (auto idx = 0UL; idx < cnt; ++idx) {
      new (&dst[idx]) T{static_cast<T&&>(src[idx])};
    }
  }
}

template <class T>
inline void uninit_move(T* src, T* dst, usize cnt) {
  if constexpr (__is_trivially_copyable(T)) {
    cnt ? (void)__builtin_memcpy(dst, src, sizeof(T) * cnt) : (void)0;
  } else {
    for (auto idx = 0UL; idx < cnt; ++idx) {
      new (&dst[idx]) T{static_cast<T&&>(src[idx])};
    }
  }
}

template <class T>
inline void shift_elements_left(T* src, usize len, usize offset) {
  if (len == 0 || offset == 0) {
    return;
  }

  const auto dst = src - offset;
  if (offset >= len) {
    return uninit_move(src, dst, len);
  }

  if constexpr (__is_trivially_copyable(T)) {
    __builtin_memmove(dst, src, len * sizeof(T));
  } else {
    for (auto idx = 0UL; idx < offset; ++idx) {
      new (&dst[idx]) T{static_cast<T&&>(src[idx])};
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
inline void shift_elements_right(T* src, usize len, usize offset) {
  if (len == 0 || offset == 0) {
    return;
  }

  const auto dst = src + offset;
  if (offset >= len) {
    return uninit_move(src, dst, len);
  }

  if constexpr (__is_trivially_copyable(T)) {
    __builtin_memmove(dst, src, len * sizeof(T));
  } else {
    for (auto idx = len; idx > len - offset; --idx) {
      new (&dst[idx - 1]) T{static_cast<T&&>(src[idx - 1])};
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
