#pragma once

#include "alloc.h"

namespace sfc::boxed {

template <class T>
class Box {
  ptr::Unique<T> _ptr{};

  explicit Box(T* ptr) noexcept : _ptr{ptr} {}

 public:
  Box() noexcept = default;

  Box(mem::inplace_t, auto&&... args) {
    auto a = alloc::Global{};

    try {
      _ptr._ptr = a.template alloc_one<T>();
      new (mem::inplace_t{}, _ptr._ptr) T{static_cast<decltype(args)&&>(args)...};
    } catch (...) {
      if (_ptr._ptr) {
        a.dealloc_one(_ptr._ptr);
        _ptr = {};
      }
    }
  }

  ~Box() {
    if (!_ptr._ptr) {
      return;
    }
    _ptr._ptr->~T();
    alloc::Global{}.dealloc_one(_ptr._ptr);
  }

  Box(Box&& other) noexcept = default;

  Box& operator=(Box&& other) noexcept {
    auto tmp = static_cast<Box&&>(other);
    mem::swap(_ptr, tmp._ptr);
    return *this;
  }

  static auto xnew(auto&&... args) -> Box {
    return Box{mem::inplace_t{}, static_cast<decltype(args)&&>(args)...};
  }

  static auto from_raw(T* ptr) -> Box {
    return Box{ptr};
  }

  operator bool() const noexcept {
    return _ptr._ptr != nullptr;
  }

  auto ptr() const -> T* {
    return _ptr._ptr;
  }

  auto operator->() const -> const T* {
    assert_fmt(_ptr._ptr != nullptr, "boxed::Box::->: deref null");
    return _ptr._ptr;
  }

  auto operator->() -> T* {
    assert_fmt(_ptr._ptr != nullptr, "boxed::Box::->: deref null");
    return _ptr._ptr;
  }

  auto operator*() const -> const T& {
    assert_fmt(_ptr._ptr != nullptr, "boxed::Box::*: deref null");
    return *_ptr._ptr;
  }

  auto operator*() -> T& {
    assert_fmt(_ptr._ptr != nullptr, "boxed::Box::*: deref null");
    return *_ptr._ptr;
  }

  auto into_raw() && -> T* {
    const auto p = _ptr._ptr;
    _ptr._ptr = nullptr;
    return p;
  }
};

template <>
class Box<void> {
  struct Meta {
    usize _size;
    void (*_dtor)(void*);

    template <class X>
    static auto from(const X*) -> const Meta& {
      static const auto res = Meta{
          ._size = sizeof(X),
          ._dtor = [](void* p) { static_cast<X*>(p)->~X(); },
      };
      return res;
    }
  };

  struct FatPtr {
    void* _self = nullptr;
    const Meta* _meta = nullptr;

    FatPtr() = default;

    FatPtr(auto* x) noexcept : _self{x}, _meta{Meta::from(x)} {}

    auto size() const {
      return _meta ? _meta->_size : 0U;
    }

    void drop() {
      _meta->_dtor(_self);
    }
  };

  FatPtr _ptr;

 public:
  template <class X>
  Box(Box<X> box) : _ptr{box.into_raw()} {}

  ~Box() {
    if (!_ptr._self) {
      return;
    }
    _ptr.drop();
    alloc::Global::dealloc_imp(_ptr._self, {_ptr.size(), 0U});
  }

  Box(Box&& other) noexcept : _ptr{other._ptr} {
    other._ptr = {};
  }

  operator bool() const noexcept {
    return _ptr._self != nullptr;
  }
};

template <class T>
class Box<T&> {
  T _ptr;

 public:
  Box() noexcept = default;

  template <class X>
  explicit Box(Box<X> box) noexcept : _ptr{mem::move(box).into_raw()} {}

  Box(Box&& other) noexcept : _ptr{other._ptr} {
    other._ptr = {};
  }

  ~Box() {
    if (!_ptr._self) {
      return;
    }
    (_ptr._meta->_dtor)(_ptr._self);
    alloc::Global{}.dealloc_imp(_ptr._self, {_ptr._meta->_size, 1U});
  }

  template <class U>
  static auto xnew(U val) -> Box {
    return Box{Box<U>{mem::inplace_t{}, static_cast<U&&>(val)}};
  }

  auto operator->() -> T* {
    return &_ptr;
  }
};

template <class R, class... T>
class Box<R(T...)> {
  using Inn = ops::Fn<R(T...)>;
  Inn _ptr = {};

 public:
  Box() noexcept = default;

  template <class X>
  Box(Box<X> box) noexcept : _ptr{static_cast<Box<X>&&>(box).into_raw()} {}

  ~Box() {
    if (!_ptr._self) {
      return;
    }

    (_ptr._meta->_dtor)(_ptr._self);
    alloc::Global{}.dealloc_imp(_ptr._self, {_ptr._meta->_size, 1U});
  }

  Box(Box&& other) noexcept : _ptr{static_cast<const Inn&>(other._ptr)} {
    other._ptr._self = nullptr;
  }

  static auto xnew(auto f) -> Box {
    return Box{Box<decltype(f)>::xnew(mem::move(f))};
  }

  Box& operator=(Box&& other) noexcept {
    auto tmp = mem::move(*this);
    mem::swap(_ptr, other._ptr);
    return *this;
  }

  explicit operator bool() const {
    return _ptr._self != nullptr;
  }

  auto operator()(T... args) -> R {
    assert_fmt(_ptr._self, "Box::(): deref null object.");

    const auto p = static_cast<ops::Any*>(_ptr._self);
    const auto f = _ptr._meta->_call;
    (p->*f)(static_cast<T&&>(args)...);
  }
};

template <class X>
auto box(X x) -> Box<X> {
  return Box<X>{mem::inplace_t{}, static_cast<X&&>(x)};
}

}  // namespace sfc::boxed

namespace sfc {
using boxed::Box;
using boxed::box;
}  // namespace sfc
