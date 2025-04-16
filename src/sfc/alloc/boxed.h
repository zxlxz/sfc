#pragma once

#include "alloc.h"

namespace sfc::boxed {

template <class T>
class Box {
  T* _ptr = nullptr;

 public:
  Box() noexcept = default;

  ~Box() {
    if (_ptr != nullptr) {
      delete _ptr;
    }
  }

  Box(Box&& other) noexcept : _ptr{mem::take(other._ptr)} {
  }

  Box& operator=(Box&& other) noexcept {
    auto tmp = static_cast<Box&&>(other);
    mem::swap(_ptr, tmp._ptr);
    return *this;
  }

  static auto xnew(auto&&... args) -> Box {
    auto res = Box{};
    res._ptr = new T{static_cast<decltype(args)&&>(args)...};
    return res;
  }

  static auto from_raw(T* ptr) -> Box {
    auto res = Box{};
    res._ptr = ptr;
    return res;
  }

  operator bool() const noexcept {
    return _ptr != nullptr;
  }

  auto ptr() const -> T* {
    return _ptr;
  }

  auto operator->() const -> const T* {
    panicking::assert_fmt(_ptr != nullptr, "boxed::Box::->: deref null");
    return _ptr;
  }

  auto operator->() -> T* {
    panicking::assert_fmt(_ptr != nullptr, "boxed::Box::->: deref null");
    return _ptr;
  }

  auto operator*() const -> const T& {
    panicking::assert_fmt(_ptr != nullptr, "boxed::Box::*: deref null");
    return *_ptr;
  }

  auto operator*() -> T& {
    panicking::assert_fmt(_ptr != nullptr, "boxed::Box::*: deref null");
    return *_ptr;
  }

  auto into_raw() && -> T* {
    return mem::take(_ptr);
  }
};

template <class T>
class Box<T&> {
  T _dyn = {};

 public:
  Box() noexcept = default;

  template <class X>
  Box(Box<X> box) noexcept : _dyn{*mem::move(box).into_raw()} {}

  Box(Box&& other) noexcept : _dyn{other._dyn} {
    other._dyn = {};
  }

  ~Box() {
    if (!_dyn._self) {
      return;
    }
    (_dyn._meta->_drop)(_dyn._self);
    delete reinterpret_cast<char*>(_dyn._self);
  }

  void operator=(Box&& other) noexcept {
    auto tmp = static_cast<Box&&>(other);
    mem::swap(_dyn, other._dyn);
  }

  auto operator->() -> T* {
    return &_dyn;
  }
};

template <class R, class... T>
class Box<R(T...)> {
  using Dyn = ops::IFn<R(T...)>;
  Dyn _dyn;

 public:
  Box() noexcept = default;

  template <class X>
  Box(Box<X> box) noexcept : _dyn{*mem::move(box).into_raw()} {}

  ~Box() {
    if (!_dyn._self) {
      return;
    }

    (_dyn._meta->_drop)(_dyn._self);
    delete static_cast<char*>(_dyn._self);
  }

  Box(Box&& other) noexcept : _dyn{other._dyn} {
    other._dyn = {};
  }

  Box& operator=(Box&& other) noexcept {
    auto tmp = mem::move(*this);
    mem::swap(_dyn, other._dyn);
    return *this;
  }

  static auto xnew(auto f) -> Box {
    return Box{Box<decltype(f)>::xnew(mem::move(f))};
  }

  explicit operator bool() const {
    return _dyn._self != nullptr;
  }

  auto operator*() -> Dyn& {
    return _dyn;
  }
};

template <class X>
auto box(X x) -> Box<X> {
  return Box<X>::xnew(static_cast<X&&>(x));
}

}  // namespace sfc::boxed

namespace sfc {
using boxed::Box;
using boxed::box;
}  // namespace sfc
