#pragma once

#include "sfc/core/mem.h"

namespace sfc::ptr {

using nullptr_t = decltype(nullptr);

template <class T>
struct Unique {
  T* _ptr = nullptr;

 public:
  Unique(T* ptr = nullptr) noexcept : _ptr{ptr} {}
  ~Unique() noexcept = default;

  Unique(const Unique&) noexcept = default;
  Unique& operator=(const Unique&) noexcept = default;

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

template <class T>
inline void write(T* dst, auto&&... args) noexcept {
  new (dst) T{static_cast<decltype(args)&&>(args)...};
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
  if (cnt == 0) {
    return;
  }
  if constexpr (__is_trivially_copyable(T)) {
    __builtin_memcpy(dst, src, sizeof(T) * cnt);
  } else {
    for (auto idx = 0UL; idx < cnt; ++idx) {
      new (&dst[idx]) T{static_cast<T&&>(src[idx])};
      src[idx].~T();
    }
  }
}

template <class T>
inline void shift_elements_left(T* src, usize len, usize offset) {
  if (len == 0 || offset == 0) {
    return;
  }

  const auto dst = src - offset;
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

template <class T>
inline auto pop_front(T* ptr, usize len) -> T {
  auto res = static_cast<T&&>(ptr[0]);
  if constexpr (__is_trivially_copyable(T)) {
    if (len > 1) {
      __builtin_memmove(ptr, ptr + 1, (len - 1) * sizeof(T));
    }
  } else {
    for (auto idx = 1UL; idx < len; ++idx) {
      ptr[idx - 1] = static_cast<T&&>(ptr[idx]);
    }
  }
  ptr[len - 1].~T();
  return res;
}

template <class T>
inline void push_front(T* ptr, usize len, auto&& val) {
  if (len == 0) {
    new (ptr) T{static_cast<decltype(val)&&>(val)};
    return;
  }

  if constexpr (__is_trivially_copyable(T)) {
    __builtin_memmove(ptr + 1, ptr, len * sizeof(T));
  } else {
    new (&ptr[len]) T{static_cast<T&&>(ptr[len - 1])};
    for (auto idx = len - 1; idx > 0; --idx) {
      ptr[idx] = static_cast<T&&>(ptr[idx - 1]);
    }
  }
  ptr[0] = static_cast<decltype(val)&&>(val);
}

}  // namespace sfc::ptr
