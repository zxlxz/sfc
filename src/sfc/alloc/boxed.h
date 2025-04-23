#pragma once

#include "sfc/alloc/alloc.h"

namespace sfc::boxed {

template <class T>
class Box {
  T* _ptr = nullptr;

 public:
  Box() noexcept = default;

  ~Box() {
    if (_ptr == nullptr) {
      return;
    }
    delete _ptr;
  }

  Box(Box&& other) noexcept : _ptr{other._ptr} {
    other._ptr = nullptr;
  }

  auto operator=(Box&& other) noexcept -> Box& {
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

  explicit operator bool() const noexcept {
    return _ptr != nullptr;
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

  void fmt(auto& f) const {
    if (_ptr == nullptr) {
      f.write_str("Box()");
    } else {
      f.write_fmt("Box({})", *_ptr);
    }
  }
};

template <class R, class... T>
class Box<R(T...)> {
  struct Meta {
    void (*_dtor)(void*) = nullptr;
    R (*_call)(void*, T&&...) = nullptr;
  };

  void*       _self = nullptr;
  const Meta* _meta = nullptr;

 public:
  Box() noexcept = default;

  ~Box() {
    if (!_self) {
      return;
    }

    (_meta->_dtor)(_self);
  }

  Box(Box&& other) noexcept : _self{other._self}, _meta{other._meta} {
    other._self = nullptr;
    other._meta = nullptr;
  }

  auto operator=(Box&& other) noexcept -> Box& {
    auto tmp = mem::move(other);
    _self = tmp._self;
    _meta = tmp._meta;
    return *this;
  }

  template <class X>
  static auto xnew(X fun) -> Box {
    static const auto META = Meta{
        ._dtor = [](void* p) { delete static_cast<X*>(p); },
        ._call = [](void* p, T&&... t) { return (*static_cast<X*>(p))(static_cast<T&&>(t)...); },
    };

    auto res = Box{};
    res._self = new X{mem::move(fun)};
    res._meta = &META;
    return res;
  }

  explicit operator bool() const {
    return _self != nullptr;
  }

  auto operator()(T... args) -> auto {
    panicking::assert_fmt(_self != nullptr, "boxed::Box::*: deref null");
    return (_meta->_call)(_self, static_cast<T&&>(args)...);
  }
};

template <class T>
class Box<T&> {
  T _impl;

 public:
  Box() noexcept = default;

  ~Box() {
    if (!_impl._self) {
      return;
    }
    (_impl._meta->_dtor)(_impl._self);
  }

  Box(Box&& other) noexcept : _impl{other._impl} {
    other._impl = {};
  }

  auto operator=(Box&& other) noexcept -> Box& {
    auto tmp = static_cast<Box&&>(other);
    mem::swap(_impl, tmp._impl);
    return *this;
  }

  template <class X>
  static auto xnew(X obj) -> Box {
    auto res = Box{};
    res._impl._self = new X{static_cast<X&&>(obj)};
    res._impl._meta = &T::template META<X>;
    return res;
  }

  explicit operator bool() const {
    return _impl._self != nullptr;
  }

  auto operator->() -> T* {
    panicking::assert_fmt(_impl._self != nullptr, "boxed::Box::->: deref null");
    return &_impl;
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
