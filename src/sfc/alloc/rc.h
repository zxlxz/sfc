#pragma once

#include "sfc/alloc/alloc.h"

namespace sfc::rc {

template <class T>
class [[nodiscard]] Rc {
  friend class option::Inner<Rc<T>>;
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
      __atomic_fetch_add(&_ptr->_cnt, 1, 0);
    }
    auto res = Rc{};
    res._ptr = _ptr;
    return res;
  }
};

}  // namespace sfc::rc

namespace sfc::option {
template <class T>
class Inner<rc::Rc<T>> {
  rc::Rc<T> _val{};

 public:
  Inner() noexcept = default;
  explicit Inner(rc::Rc<T>&& val) noexcept : _val{static_cast<rc::Rc<T>&&>(val)} {}
  ~Inner() noexcept = default;

  Inner(Inner&&) noexcept = default;

  auto is_some() const noexcept -> bool {
    return _val._ptr != nullptr;
  }

  auto is_none() const noexcept -> bool {
    return _val._ptr == nullptr;
  }

  auto operator*() const noexcept -> const rc::Rc<T>& {
    return _val;
  }

  auto operator*() noexcept -> rc::Rc<T>& {
    return _val;
  }
};
}  // namespace sfc::option

namespace sfc {
using rc::Rc;
}  // namespace sfc
