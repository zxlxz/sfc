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

    const auto p = a.template alloc_one<T>();
    try {
      new (mem::inplace_t{}, p) T{static_cast<decltype(args)&&>(args)...};
      _ptr = ptr::Unique<T>{p};
    } catch (...) {
      a.dealloc_one(p);
      throw;
    }
  }

  ~Box() {
    if (!_ptr) {
      return;
    }
    _ptr->~T();
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

  [[sfc_inline]] operator bool() const noexcept {
    return _ptr._ptr != nullptr;
  }

  [[sfc_inline]] auto ptr() const -> T* {
    return _ptr._ptr;
  }

  [[sfc_inline]] auto operator->() const -> const T* {
    assert_fmt(_ptr._ptr != nullptr, "boxed::Box::->: deref null");
    return _ptr._ptr;
  }

  [[sfc_inline]] auto operator->() -> T* {
    assert_fmt(_ptr._ptr != nullptr, "boxed::Box::->: deref null");
    return _ptr._ptr;
  }

  [[sfc_inline]] auto operator*() const -> const T& {
    assert_fmt(_ptr._ptr != nullptr, "boxed::Box::*: deref null");
    return *_ptr._ptr;
  }

  [[sfc_inline]] auto operator*() -> T& {
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
class Box<Any&> {
  struct Meta {
    usize _size;
    void (*_dtor)(void*);

    template <class X>
    static auto of(const X&) -> const Meta& {
      static const auto res = Meta{
          ._size = sizeof(X),
          ._dtor = [](void* p) { static_cast<X*>(p)->~X(); },
      };
      return res;
    }
  };

  Any* _self = nullptr;
  const Meta* _meta = nullptr;

  template <class X>
  Box(mem::inplace_t, X& x) : _self{reinterpret_cast<Any*>(&x)}, _meta{&Meta::of(x)} {}

 public:
  template <class X>
  Box(Box<X> box) : Box{mem::inplace_t{}, *(box.into_raw())} {}

  ~Box() {}

  Box(Box&& other) noexcept : _self{other._self}, _meta{other._meta} {
    _self = nullptr;
  }
};

template <class T>
class Box<T&> {
  T _inn = {};

  Box(mem::inplace_t, auto& x) : _inn{T::from(x)} {}

 public:
  Box() noexcept = default;

  template <class X>
  Box(Box<X> box) : Box{mem::inplace_t{}, *mem::move(box).into_raw()} {}

  Box(Box&& other) noexcept : _inn{other._inn} {
    other._inn._self = nullptr;
  }

  ~Box() {
    if (!_inn._self) {
      return;
    }
    (_inn._meta->_dtor)(_inn._self);
    alloc::Global{}.dealloc_imp(_inn._self, {_inn._meta->_size, 1U});
  }

  auto operator->() -> T* {
    return &_inn;
  }
};

template <class R, class... T>
class Box<R(T...)> {
  using Inn = ops::Fn<R(T...)>;
  Inn _inn = {};

  explicit Box(mem::inplace_t, auto& x) noexcept : _inn{Inn::from(x)} {}

 public:
  Box() = default;

  ~Box() {
    if (!_inn._self) {
      return;
    }

    (_inn._meta->_dtor)(_inn._self);
    alloc::Global{}.dealloc_imp(_inn._self, {_inn._meta->_size, 1U});
  }

  Box(Box&& other) noexcept : _inn{static_cast<const Inn&>(other._inn)} {
    other._inn._self = nullptr;
  }

  template <class X>
  static auto xnew(X x) -> Box {
    auto ptr = Box<X>::xnew(static_cast<X&&>(x)).into_raw();
    return Box{mem::inplace_t{}, *ptr};
  }

  auto operator=(Box&& other) noexcept -> Box& {
    auto tmp = mem::move(*this);
    mem::swap(_inn, other._inn);
    return *this;
  }

  explicit operator bool() const {
    return _inn._self != nullptr;
  }

  auto operator()(T... args) -> R {
    assert_fmt(_inn._self, "Box::(): deref null object.");

    const auto f = _inn._meta->_call;
    (_inn._self->*f)(static_cast<T&&>(args)...);
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
