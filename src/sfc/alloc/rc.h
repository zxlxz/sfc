#pragma once

#include "sfc/core.h"

namespace sfc::rc {

template <class T>
struct RcBox {
  T   _val;
  i32 _strong = 1;
  i32 _weak = 1;

 public:
  [[nodiscard]] auto strong() const -> i32 {
    return _strong;
  }

  [[nodiscard]] auto weak() const -> i32 {
    return _weak;
  }

  auto inc_strong() -> i32 {
    return __atomic_add_fetch(&_strong, 1, __ATOMIC_RELAXED);
  }

  auto dec_strong() -> i32 {
    return __atomic_add_fetch(&_strong, -1, __ATOMIC_RELAXED);
  }

  auto inc_weak() -> i32 {
    return __atomic_add_fetch(&_weak, 1, __ATOMIC_RELAXED);
  }

  auto dec_weak() -> i32 {
    return __atomic_add_fetch(&_weak, -1, __ATOMIC_RELAXED);
  }
};

template <class T>
class Weak;

template <class T>
class Rc {
  RcBox<T>* _ptr = nullptr;

 public:
  Rc() noexcept = default;

  ~Rc() {
    if (!_ptr) {
      return;
    }

    if (_ptr->dec_strong() == 0) {
      mem::drop(*_ptr);
      if (_ptr->dec_weak() == 0) {
        ::delete reinterpret_cast<char*>(_ptr);
      }
    }
  }

  Rc(Rc&& other) noexcept : _ptr{other._ptr} {
    other._ptr = nullptr;
  }

  auto operator=(Rc&& other) noexcept -> Rc& {
    auto tmp = static_cast<Rc&&>(other);
    mem::swap(_ptr, tmp._ptr);
    return *this;
  }

  static auto from_raw(RcBox<T>* ptr) -> Rc {
    auto res = Rc{};
    res._ptr = ptr;
    return res;
  }

  template <class... U>
  static auto xnew(U&&... args) -> Rc {
    return Rc::from_raw(new RcBox{T{static_cast<U&&>(args)...}});
  }

  auto clone() const -> Rc {
    if (_ptr) {
      _ptr->inc_strong();
    }
    return Rc::from_raw(_ptr);
  }

  explicit operator bool() const {
    return _ptr != nullptr;
  }

  auto operator->() const -> const T* {
    panicking::assert_fmt(_ptr != nullptr, "Rc::operator->: deref null");
    return &_ptr->_val;
  }

  auto operator->() -> T* {
    panicking::assert_fmt(_ptr != nullptr, "Rc::operator->: deref null");
    return &_ptr->_val;
  }

  auto operator*() const -> const T& {
    panicking::assert_fmt(_ptr != nullptr, "Rc::operator*: deref null");
    return _ptr->_val;
  }

  auto operator*() -> T& {
    panicking::assert_fmt(_ptr != nullptr, "Rc::operator*: deref null");
    return _ptr->_val;
  }

  auto weak() -> Weak<T>;
};

template <class T>
class Weak {
  RcBox<T>* _ptr = nullptr;

 public:
  Weak() noexcept = default;

  ~Weak() {
    if (!_ptr) {
      return;
    }

    if (_ptr->dec_weak() == 0) {
      alloc::Global{}.dealloc_one(_ptr);
    }
  }

  Weak(Weak&& other) noexcept : _ptr{other._ptr} {
    other._ptr = nullptr;
  }

  auto operator=(Weak&& other) noexcept -> Weak& {
    auto tmp = static_cast<Weak&&>(other);
    mem::swap(_ptr, tmp._ptr);
    return *this;
  }

  auto clone() -> Weak {
    _ptr->add_weak();
    return Weak{_ptr};
  }

  auto weak_count() const -> usize {
    if (!_ptr) {
      return 0U;
    }

    const auto res = _ptr->weak();
    return static_cast<usize>(res);
  }
};

template <class T>
auto Rc<T>::weak() -> Weak<T> {
  _ptr->add_weak();
  return {_ptr};
}

}  // namespace sfc::rc

namespace sfc {
using rc::Rc;
}  // namespace sfc
