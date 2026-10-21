#pragma once

#include "sfc/core.h"

namespace sfc::boxed {

template <class T>
class [[nodiscard]] Box {
  T* _ptr{nullptr};

 public:
  Box() noexcept = default;

  ~Box() noexcept {
    if (!_ptr) return;
    delete _ptr;
  }

  Box(Box&& other) noexcept : _ptr{mem::take(other._ptr)} {}

  Box& operator=(Box&& other) noexcept {
    if (this != &other) {
      mem::swap(_ptr, other._ptr);
    }
    return *this;
  }

  static auto from_raw(T* ptr) noexcept -> Box {
    auto res = Box{};
    res._ptr = ptr;
    return res;
  }

  static auto new_(auto&&... args) -> Box {
    auto res = Box{};
    res._ptr = new T{(decltype(args)&&)(args)...};
    return res;
  }

 public:
  auto is_null() const noexcept -> bool {
    return _ptr == nullptr;
  }

  auto as_ptr() const noexcept -> T* {
    return _ptr;
  }

  auto into_raw() && noexcept -> T* {
    return mem::take(_ptr);
  }

 public:
  // trait: Deref<const T*>
  auto operator->() const noexcept -> const T* {
    return _ptr;
  }

  // trait: Deref<T*>
  auto operator->() noexcept -> T* {
    return _ptr;
  }

  // trait: Deref<const T&>
  auto operator*() const noexcept -> const T& {
    return *_ptr;
  }

  // trait: Deref<T&>
  auto operator*() noexcept -> T& {
    return *_ptr;
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    if (_ptr == nullptr) {
      f.write_str("Box()");
    } else {
      f.write_fmt("Box({})", *_ptr);
    }
  }
};

template <class R, class... T>
class [[nodiscard]] Box<R(T...)> {
  struct VTbl {
    R (*call)(void*, T&&...);
    void (*dtor)(void*);
  };

  template <class X>
  static constexpr auto vtbl = VTbl{
      .call = [](void* x, T&&... u) -> R { return (*(X*)(x))((T&&)(u)...); },
      .dtor = [](void* x) { delete (X*)(x); },
  };

  const VTbl* _vptr;
  void* _impl;

 public:
  Box() noexcept : _vptr{nullptr}, _impl{nullptr} {}

  ~Box() noexcept {
    if (_impl == nullptr) {
      return;
    }
    (_vptr->dtor)(_impl);
  }

  Box(Box&& other) noexcept : _vptr{mem::take(other._vptr)}, _impl{mem::take(other._impl)} {}

  Box& operator=(Box&& other) noexcept {
    if (this != &other) {
      mem::swap(_vptr, other._vptr);
      mem::swap(_impl, other._impl);
    }
    return *this;
  }

  template <class X>
  static auto new_(X impl) noexcept -> Box {
    auto res = Box{};
    res._vptr = &Box::vtbl<X>;
    res._impl = new X{mem::move(impl)};
    return res;
  }

 public:
  auto operator()(T... args) -> R {
    return (_vptr->call)(_impl, (T&&)(args)...);
  }
};

template <class T>
auto box(T val) -> Box<T> {
  return Box<T>::new_(mem::move(val));
}

}  // namespace sfc::boxed

namespace sfc {
using boxed::Box;
}  // namespace sfc
