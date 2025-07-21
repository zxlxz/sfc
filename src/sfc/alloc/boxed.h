#pragma once

#include "sfc/alloc/alloc.h"

namespace sfc::boxed {

template <class T>
class Box {
  T* _ptr = nullptr;

 public:
  Box() noexcept = default;

  ~Box() {
    if (_ptr) {
      delete _ptr;
    }
  }

  Box(Box&& other) noexcept : _ptr{mem::take(other._ptr)} {}

  auto operator=(Box&& other) noexcept -> Box& {
    if (this != &other) {
      if (_ptr) {
        delete _ptr;
      }
      _ptr = mem::take(other._ptr);
    }
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

  auto ptr() const -> T* {
    return _ptr;
  }

  auto operator->() const -> const T* {
    panicking::assert(_ptr != nullptr, "boxed::Box::->: deref null");
    return _ptr;
  }

  auto operator->() -> T* {
    panicking::assert(_ptr != nullptr, "boxed::Box::->: deref null");
    return _ptr;
  }

  auto operator*() const -> const T& {
    panicking::assert(_ptr != nullptr, "boxed::Box::*: deref null");
    return *_ptr;
  }

  auto operator*() -> T& {
    panicking::assert(_ptr != nullptr, "boxed::Box::*: deref null");
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

    template <class X>
    static auto of() -> Meta {
      if constexpr (__is_trivially_copyable(X) && sizeof(X) <= sizeof(void*)) {
        const auto call = +[](void* x, T&&... t) -> R {  //
          return (*static_cast<X*>(static_cast<void*>(&x)))(static_cast<T&&>(t)...);
        };
        return Meta{nullptr, call};
      }

      const auto dtor = +[](void* p) { delete static_cast<X*>(p); };
      const auto call = +[](void* p, T&&... t) -> R {  //
        return (*static_cast<X*>(p))(static_cast<T&&>(t)...);
      };
      return Meta{dtor, call};
    }
  };

  const Meta* _meta = nullptr;
  void* _data = nullptr;

 public:
  Box() noexcept = default;

  ~Box() {
    if (_meta && _meta->_dtor) {
      (_meta->_dtor)(_data);
    }
  }

  Box(Box&& other) noexcept : _meta{other._meta}, _data{other._data} {
    other._meta = nullptr;
  }

  Box& operator=(Box&& other) noexcept {
    if (this != &other) {
      if (_meta && _meta->_dtor) {
        (_meta->_dtor)(_data);
      }
      _meta = mem::take(other._meta);
      _data = mem::take(other._data);
    }

    return *this;
  }

  template <class X>
  static auto xnew(X fun) -> Box {
    static const auto meta = Meta::template of<X>();
    auto res = Box{};
    res._meta = &meta;
    if (!meta._dtor) {
      __builtin_memcpy(&res._data, &fun, sizeof(X));
    } else {
      res._data = new X{static_cast<X&&>(fun)};
    }
    return res;
  }

  explicit operator bool() const {
    return _meta != nullptr;
  }

  auto operator()(T... args) -> auto {
    panicking::assert(_meta != nullptr, "boxed::Box::*: deref null");
    return (_meta->_call)(_data, static_cast<T&&>(args)...);
  }
};

template <class T>
class Box<T&> {
  T _impl;

 public:
  Box() noexcept = default;

  ~Box() {
    if (_impl._meta && _impl._meta->_dtor) {
      (_impl._meta->_dtor)(_impl._self);
    }
  }

  Box(Box&& other) noexcept : _impl{mem::take(other._impl)} {}

  auto operator=(Box&& other) noexcept -> Box& {
    if (this != &other) {
      if (_impl._meta && _impl._meta->_dtor) {
        (_impl._meta->_dtor)(_impl._self);
      }
      _impl = mem::take(other._impl);
    }
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
    panicking::assert(_impl._self != nullptr, "boxed::Box::->: deref null");
    return &_impl;
  }
};

template <class X>
auto box(X x) -> Box<X> {
  return Box<X>::xnew(static_cast<X&&>(x));
}

}  // namespace sfc::boxed
