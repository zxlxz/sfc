#pragma once

#include "mem.h"

namespace sfc::ptr {

template <class T>
struct Unique {
  T* _ptr = nullptr;

 public:
  [[sfc_inline]] Unique() noexcept = default;

  [[sfc_inline]] Unique(decltype(nullptr)) noexcept {}

  [[sfc_inline]] explicit Unique(T* ptr) noexcept : _ptr{ptr} {}

  [[sfc_inline]] ~Unique() noexcept {
    _ptr = nullptr;
  }

  [[sfc_inline]] Unique(Unique&& other) noexcept : _ptr{other._ptr} {
    other._ptr = nullptr;
  }

  [[sfc_inline]] Unique& operator=(Unique&& other) noexcept {
    _ptr = other._ptr;
    other._ptr = nullptr;
    return *this;
  }

  [[sfc_inline]] explicit operator bool() const {
    return _ptr != nullptr;
  }

  [[sfc_inline]] auto ptr() -> T* {
    return _ptr;
  }

  [[sfc_inline]] auto operator->() const -> const T* {
    assert_fmt(_ptr != nullptr, "ptr::Unique::->: deref null");
    return _ptr;
  }

  [[sfc_inline]] auto operator->() -> T* {
    assert_fmt(_ptr != nullptr, "ptr::Unique::->: deref null");
    return _ptr;
  }

  [[sfc_inline]] auto operator*() const -> const T& {
    assert_fmt(_ptr != nullptr, "ptr::Unique::*: deref null");
    return *_ptr;
  }

  [[sfc_inline]] auto operator*() -> T& {
    assert_fmt(_ptr != nullptr, "ptr::Unique::*: deref null");
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
[[sfc_inline]] inline void write(T* dst, T val) {
  new (mem::inplace_t{}, dst) T{static_cast<T&&>(val)};
}

template <class T>
[[sfc_inline]] inline void write_bytes(T* dst, u8 val, usize cnt) {
  if (cnt == 0) {
    return;
  }
  __builtin_memset(dst, val, cnt * sizeof(T));
}

template <class T>
[[sfc_inline]] inline void drop([[maybe_unused]] T* ptr, [[maybe_unused]] usize cnt) {
  if constexpr (__is_trivially_copyable(T)) {
    (void)ptr;
    (void)cnt;
  } else {
    for (const auto end = ptr + cnt; ptr != end; ++ptr) {
      ptr->~T();
    }
  }
}

template <class T>
[[sfc_inline]] inline void copy(const T* src, T* dst, usize cnt) {
  if (cnt == 0) {
    return;
  }

  if constexpr (__is_trivially_copyable(T)) {
    __builtin_memmove(dst, src, cnt * sizeof(T));
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
[[sfc_inline]] inline void copy_nonoverlapping(const T* src, T* dst, usize cnt) {
  if (cnt == 0) {
    return;
  }
  if constexpr (__is_trivially_copyable(T)) {
    __builtin_memcpy(dst, src, cnt * sizeof(T));
  } else {
    for (const auto end = src + cnt; src != end; ++src, ++dst) {
      *dst = *src;
    }
  }
}

template <class T>
[[sfc_inline]] inline void move(T* src, T* dst, usize cnt) {
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
[[sfc_inline]] inline void uninit_move(T* src, T* dst, usize cnt) {
  if (cnt == 0) {
    return;
  }
  if constexpr (__is_trivially_copyable(T)) {
    __builtin_memcpy(dst, src, sizeof(T) * cnt);
  } else {
    for (const auto end = src + cnt; src != end; ++src, ++dst) {
      new (mem::inplace_t{}, dst) T{static_cast<T&&>(*src)};
    }
  }
}

template <class T>
[[sfc_inline]] inline void uninit_copy(const T* src, T* dst, usize cnt) {
  if (cnt == 0) {
    return;
  }

  if constexpr (__is_trivially_copyable(T)) {
    __builtin_memcpy(dst, src, sizeof(T) * cnt);
  } else {
    for (const auto end = src + cnt; src != end; ++src, ++dst) {
      new (mem::inplace_t{}, dst) T{src->clone()};
    }
  }
}

}  // namespace sfc::ptr
