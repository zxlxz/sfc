#pragma once

#include "sfc/core.h"

namespace sfc::sync {

template <class T>
class [[nodiscard]] Arc {
  struct Inn {
    sync::Atomic<u32> _cnt;
    T _val;

    auto inc_count() noexcept -> u32 {
      return _cnt.fetch_add(1, sync::Ordering::Relaxed);
    }

    auto dec_count() noexcept -> u32 {
      return _cnt.fetch_sub(1, sync::Ordering::Release);
    }
  };
  Inn* _ptr{nullptr};

 public:
  Arc() noexcept = default;

  ~Arc() noexcept {
    if (_ptr == nullptr) return;

    if (_ptr->dec_count() == 1) {
      delete _ptr;
    }
  }

  Arc(Arc&& other) noexcept : _ptr{mem::take(other._ptr)} {}

  Arc& operator=(Arc&& other) noexcept {
    if (this == &other) return *this;
    mem::swap(_ptr, other._ptr);
    return *this;
  }

  static auto new_(auto&&... args) -> Arc {
    auto res = Arc{};
    res._ptr = new Inn{{1}, {(decltype(args)&&)(args)...}};
    return res;
  }

  auto as_ptr() const noexcept -> const T* {
    return _ptr ? &_ptr->_val : nullptr;
  }

 public:
  // trait: Deref<const T*>
  auto operator->() const noexcept -> const T* {
    return &_ptr->_val;
  }

  // trait: Deref<T*>
  auto operator->() noexcept -> T* {
    return &_ptr->_val;
  }

  // trait: Deref<const T&>
  auto operator*() const noexcept -> const T& {
    return _ptr->_val;
  }

  // trait: Deref<T&>
  auto operator*() noexcept -> T& {
    return _ptr->_val;
  }

  // trait: Clone
  auto clone() const noexcept -> Arc {
    if (_ptr == nullptr) return {};

    _ptr->inc_count();
    auto res = Arc{};
    res._ptr = _ptr;
    return res;
  }
};

}  // namespace sfc::sync

namespace sfc {
using sync::Arc;
}  // namespace sfc
