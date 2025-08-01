#pragma once

#include "sfc/core/mem.h"
#include "sfc/core/panicking.h"

namespace sfc::ptr {

using nullptr_t = decltype(nullptr);

template <class T>
struct Unique {
  T* _ptr = nullptr;

 public:
  Unique() noexcept = default;

  Unique(nullptr_t) noexcept {}

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
inline auto read(T* src) -> T {
  if constexpr (__is_trivially_copyable(T)) {
    return *src;
  } else {
    auto val = *src;
    src->~T();
    return val;
  }
}

template <class T, class... U>
inline void write(T* dst, U&&... args) {
  new (dst) T{static_cast<U&&>(args)...};
}

template <class T>
inline void drop_in_place([[maybe_unused]] T* ptr, [[maybe_unused]] usize cnt) {
  if constexpr (!__is_trivially_destructible(T)) {
    for (const auto end = ptr + cnt; ptr != end; ++ptr) {
      ptr->~T();
    }
  }
}

template <class T>
inline void copy(const T* src, T* dst, usize cnt) {
  if (cnt == 0 || src == dst) {
    return;
  }
  if constexpr (__is_trivially_copyable(T)) {
    __builtin_memmove(dst, src, cnt * sizeof(T));
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
  if (cnt == 0 || !src || !dst) {
    return;
  }
  if constexpr (__is_trivially_copyable(T)) {
    __builtin_memcpy(dst, src, cnt * sizeof(T));
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
    __builtin_memcpy(dst, src, sizeof(T) * cnt);
  } else {
    if (dst < src) {
      for (auto idx = 0UL; idx < cnt; ++idx) {
        new (dst + idx) T{src[idx]};
      }
    } else {
      for (; cnt > 0; --cnt) {
        new (dst + cnt - 1) T{src[cnt - 1]};
      }
    }
  }
}

template <class T>
inline void uninit_move(T* src, T* dst, usize cnt) {
  if (cnt == 0 || src == dst) {
    return;
  }
  if constexpr (__is_trivially_copyable(T)) {
    __builtin_memcpy(dst, src, sizeof(T) * cnt);
  } else {
    if (dst < src) {
      for (auto idx = 0UL; idx < cnt; ++idx) {
        new (&dst[idx]) T{static_cast<T&&>(src[idx])};
        src[idx].~T();
      }
    } else {
      for (; cnt > 0; --cnt) {
        new (&dst[cnt - 1]) T{static_cast<T&&>(src[cnt - 1])};
        src[cnt - 1].~T();
      }
    }
  }
}

template <class T>
inline void shift_elements(T* src, usize len, isize offset) {
  if (len == 0 || offset == 0) {
    return;
  }

  const auto dst = src + offset;
  if constexpr (__is_trivially_copyable(T)) {
    __builtin_memmove(dst, src, len * sizeof(T));
    return;
  }

  if (offset > 0) {
    const auto off = static_cast<usize>(offset);
    for (auto idx = len; idx > len - off; --idx) {
      new (&dst[idx - 1]) T{static_cast<T&&>(src[idx - 1])};
    }
    for (auto idx = len - off; idx > 0; --idx) {
      dst[idx - 1] = static_cast<T&&>(src[idx - 1]);
    }
    for (auto idx = off; idx > 0; --idx) {
      src[idx].~T();
    }
  } else {
    const auto off = static_cast<usize>(0 - offset);
    for (auto idx = 0UL; idx < off; ++idx) {
      new (&dst[idx]) T{static_cast<T&&>(src[idx])};
    }
    for (auto idx = off; idx < len; ++idx) {
      dst[idx] = static_cast<T&&>(src[idx]);
    }
    for (auto idx = len - off; idx < len; ++idx) {
      src[idx].~T();
    }
  }
}

}  // namespace sfc::ptr
