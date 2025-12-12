#pragma once

#include "sfc/alloc/alloc.h"

namespace sfc::rc {

template <class T>
class [[nodiscard]] Rc {
  struct Inn {
    T _val;
    int _cnt = 1;
  };
  Inn* _ptr = nullptr;

 public:
  Rc() noexcept = default;

  ~Rc() noexcept {
    if (_ptr && __atomic_fetch_sub(&_ptr->_cnt, 1, 0) == 1) {
      delete _ptr;
    }
  }

  Rc(Rc&& other) noexcept : _ptr{mem::take(other._ptr)} {}

  Rc& operator=(Rc&& other) noexcept {
    if (this != &other) {
      if (_ptr && __atomic_fetch_sub(&_ptr->_cnt, 1, 0) == 1) {
        delete _ptr;
      }
      _ptr = mem::take(other._ptr);
    }
    return *this;
  }

  static auto xnew(auto&&... args) noexcept -> Rc {
    auto res = Rc{};
    res._ptr = new Inn{T{static_cast<decltype(args)&&>(args)...}};
    return res;
  }

  auto clone() const noexcept -> Rc {
    if (_ptr) {
      __atomic_fetch_add(&_ptr->_cnt, 1, 0);
    }
    auto res = Rc{};
    res._ptr = _ptr;
    return res;
  }

  explicit operator bool() const noexcept {
    return _ptr != nullptr;
  }

  auto operator->() const noexcept -> const T* {
    panicking::expect(_ptr != nullptr, "Rc::operator->: deref null");
    return &_ptr->_val;
  }

  auto operator->() noexcept -> T* {
    panicking::expect(_ptr != nullptr, "Rc::operator->: deref null");
    return &_ptr->_val;
  }

  auto operator*() const noexcept -> const T& {
    panicking::expect(_ptr != nullptr, "Rc::operator*: deref null");
    return _ptr->_val;
  }

  auto operator*() noexcept -> T& {
    panicking::expect(_ptr != nullptr, "Rc::operator*: deref null");
    return _ptr->_val;
  }
};

}  // namespace sfc::rc

namespace sfc {
using rc::Rc;
}  // namespace sfc
