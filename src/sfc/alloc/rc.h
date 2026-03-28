#pragma once

#include "sfc/sync/atomic.h"

namespace sfc::rc {

template <class T>
class [[nodiscard]] Rc {
  struct Inn {
    sync::Atomic<int> _cnt{1};
    T _val;
  };
  Inn* _ptr{nullptr};

 public:
  Rc() noexcept = default;

  explicit Rc(T val) : _ptr{new Inn{1, static_cast<T&&>(val)}} {}

  ~Rc() noexcept {
    if (!_ptr) return;

#ifndef __clang_analyzer__
    const auto cnt = _ptr->_cnt.fetch_sub(1, sync::Ordering::AcqRel);
    if (cnt == 1) {
      delete _ptr;
    }
#endif
  }

  Rc(Rc&& other) noexcept : _ptr{other._ptr} {
    other._ptr = nullptr;
  }

  Rc& operator=(Rc&& other) noexcept {
    if (this != &other) {
      mem::swap(_ptr, other._ptr);
    }
    return *this;
  }

  auto as_ptr() const noexcept -> T* {
    if (!_ptr) return nullptr;
    return &_ptr->_val;
  }

  auto operator->() const noexcept -> const T* {
    return &_ptr->_val;
  }

  auto operator->() noexcept -> T* {
    return &_ptr->_val;
  }

  auto operator*() const noexcept -> const T& {
    return _ptr->_val;
  }

  auto operator*() noexcept -> T& {
    return _ptr->_val;
  }

 public:
  // trait: Clone
  auto clone() const noexcept -> Rc {
    if (!_ptr) return {};

    _ptr->_cnt.fetch_add(1, sync::Ordering::Relaxed);
    auto res = Rc{};
    res._ptr = _ptr;
    return res;
  }
};

}  // namespace sfc::rc

namespace sfc {
using rc::Rc;
}  // namespace sfc
