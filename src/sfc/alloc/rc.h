#pragma once

#include "sfc/sync/atomic.h"

namespace sfc::rc {

template <class T>
class [[nodiscard]] Rc {
 public:
  struct Inn {
    T _val;
    sync::Atomic<int> _cnt{1};
  };
  Inn* _ptr = nullptr;

 public:
  Rc() noexcept = default;

  ~Rc() noexcept {
    if (_ptr && _ptr->_cnt.fetch_sub(1) == 1) {
      delete _ptr;
    }
  }

  Rc(Rc&& other) noexcept : _ptr{mem::take(other._ptr)} {}

  Rc& operator=(Rc&& other) noexcept {
    if (this != &other) {
      mem::swap(_ptr, other._ptr);
    }
    return *this;
  }

  static auto xnew(auto&&... args) noexcept -> Rc {
    auto res = Rc{};
    res._ptr = new Inn{T{static_cast<decltype(args)&&>(args)...}};
    return res;
  }

  auto as_ptr() const noexcept -> T* {
    return _ptr ? &_ptr->_val : nullptr;
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
    if (_ptr) {
      _ptr->_cnt.fetch_add(1);
    }
    auto res = Rc{};
    res._ptr = _ptr;
    return res;
  }
};

}  // namespace sfc::rc

namespace sfc {
using rc::Rc;
}  // namespace sfc
