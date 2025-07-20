#pragma once

#include "sfc/core/mem.h"
#include "sfc/core/panicking.h"

namespace sfc::ptr {

template <class T>
struct Unique {
  T* _ptr = nullptr;

 public:
  Unique() noexcept = default;

  Unique(decltype(nullptr)) noexcept {}

  explicit Unique(T* ptr) noexcept : _ptr{ptr} {}

  ~Unique() noexcept {
    _ptr = nullptr;
  }

  Unique(Unique&& other) noexcept : _ptr{other._ptr} {
    other._ptr = nullptr;
  }

  auto operator=(Unique&& other) noexcept -> Unique& {
    _ptr = other._ptr;
    other._ptr = nullptr;
    return *this;
  }

  explicit operator bool() const {
    return _ptr != nullptr;
  }

  auto ptr() -> T* {
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

template <class T, class F>
auto cast(F* p) -> T* {
  if constexpr (__is_constructible(T*, F*)) {
    return static_cast<T*>(p);
  }
  return reinterpret_cast<T*>(p);
}

template <class T, class F>
auto cast(const F* p) -> const T* {
  if constexpr (__is_constructible(const T*, F*)) {
    return static_cast<const T*>(p);
  }
  return reinterpret_cast<const T*>(p);
}

template <class T>
auto cast_mut(const T* p) -> T* {
  return const_cast<T*>(p);
}

template <class T>
inline auto read(T* ptr) -> T {
  auto res = static_cast<T&&>(*ptr);
  ptr->~T();
  return res;
}

template <class T, class... U>
inline void write(T* dst, U&&... args) {
  new (dst) T{static_cast<U&&>(args)...};
}

template <class T>
inline void write_bytes(T* dst, u8 val, usize cnt) {
  if (cnt != 0) {
    __builtin_memset(dst, val, cnt * sizeof(T));
  }
}

template <class T>
inline void drop_in_place([[maybe_unused]] T* ptr, [[maybe_unused]] usize cnt) {
  if constexpr (!__is_trivially_copyable(T)) {
    for (const auto end = ptr + cnt; ptr != end; ++ptr) {
      ptr->~T();
    }
  }
}

template <class T>
inline void copy(const T* src, T* dst, usize cnt) {
  if constexpr (__is_trivially_copyable(T)) {
    if (cnt != 0) {
      __builtin_memmove(dst, src, cnt * sizeof(T));
    }
  } else {
    if (dst < src) {
      for (const auto end = src + cnt; src != end; ++src, ++dst) {
        *dst = *src;
      }
    } else if (dst > src) {
      src += cnt - 1;
      dst += cnt - 1;
      for (const auto end = src - cnt; src != end; --src, --dst) {
        *dst = *src;
      }
    }
  }
}

template <class T>
inline void copy_nonoverlapping(const T* src, T* dst, usize cnt) {
  if constexpr (__is_trivially_copyable(T)) {
    if (cnt != 0) {
      __builtin_memcpy(dst, src, cnt * sizeof(T));
    }
  } else {
    for (const auto end = src + cnt; src != end; ++src, ++dst) {
      *dst = *src;
    }
  }
}

template <class T>
inline void move(T* src, T* dst, usize cnt) {
  if (cnt == 0) {
    return;
  }

  if constexpr (__is_trivially_copyable(T)) {
    __builtin_memmove(dst, src, cnt * sizeof(T));
  } else {
    if (dst < src) {
      for (const auto end = src + cnt; src != end; ++src, ++dst) {
        *dst = static_cast<T&&>(*src);
      }
    } else if (dst > src) {
      src += cnt - 1;
      dst += cnt - 1;
      for (const auto end = src - cnt; src != end; --src, --dst) {
        *dst = static_cast<T&&>(*src);
      }
    }
  }
}

template <class T>
inline void uninit_move(T* src, T* dst, usize cnt) {
  if constexpr (__is_trivially_copyable(T)) {
    if (cnt != 0) {
      __builtin_memcpy(dst, src, sizeof(T) * cnt);
    }
  } else {
    for (const auto end = src + cnt; src != end; ++src, ++dst) {
      new (dst) T{static_cast<T&&>(*src)};
    }
  }
}

template <class T>
inline void uninit_copy(const T* src, T* dst, usize cnt) {
  if constexpr (__is_trivially_copyable(T)) {
    if (cnt != 0) {
      __builtin_memcpy(dst, src, sizeof(T) * cnt);
    }
  } else {
    for (const auto end = src + cnt; src != end; ++src, ++dst) {
      new (dst) T{src->clone()};
    }
  }
}

}  // namespace sfc::ptr
