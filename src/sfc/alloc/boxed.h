#pragma once

#include "sfc/alloc/alloc.h"

namespace sfc::boxed {

template <class T>
class [[nodiscard]] Box {
  T* _ptr = nullptr;

 public:
  Box() noexcept = default;

  ~Box() {
    if (_ptr) {
      delete _ptr;
    }
  }

  Box(const Box&) = delete;

  Box& operator=(const Box&) = delete;

  Box(Box&& other) noexcept : _ptr{other._ptr} {
    other._ptr = {};
  }

  Box& operator=(Box&& other) noexcept {
    if (this != &other) {
      if (_ptr) {
        delete _ptr;
      }
      _ptr = other._ptr;
      other._ptr = {};
    }
    return *this;
  }

  static auto from_raw(T* ptr) noexcept -> Box {
    auto res = Box{};
    res._ptr = ptr;
    return res;
  }

  static auto xnew(auto&&... args) -> Box {
    auto res = Box{};
    res._ptr = new T{static_cast<decltype(args)&&>(args)...};
    return res;
  }

  explicit operator bool() const noexcept {
    return _ptr != nullptr;
  }

  auto ptr() const noexcept -> T* {
    return _ptr;
  }

  auto into_raw() && noexcept -> T* {
    const auto res = _ptr;
    _ptr = nullptr;
    return res;
  }

  template <trait::polymorphic_ B>
  auto cast() && noexcept -> Box<B> {
    const auto p = static_cast<B*>(mem::take(_ptr));
    return Box<B>::from_raw(p);
  }

  auto operator->() const noexcept -> const T* {
    panicking::expect(_ptr != nullptr, "boxed::Box::->: deref null");
    return _ptr;
  }

  auto operator->() noexcept -> T* {
    panicking::expect(_ptr != nullptr, "boxed::Box::->: deref null");
    return _ptr;
  }

  auto operator*() const noexcept -> const T& {
    panicking::expect(_ptr != nullptr, "boxed::Box::*: deref null");
    return *_ptr;
  }

  auto operator*() noexcept -> T& {
    panicking::expect(_ptr != nullptr, "boxed::Box::*: deref null");
    return *_ptr;
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
class [[nodiscard]] Box<R(T...)> {
  using dtor_t = void (*)(void*);
  using call_t = R (*)(void*, T&&...);
  struct Meta {
    dtor_t _dtor = nullptr;
    call_t _call = nullptr;
  };

  const Meta* _meta{nullptr};
  void* _data{nullptr};

 public:
  Box() noexcept = default;

  ~Box() noexcept {
    if (_data) {
      (_meta->_dtor)(_data);
    }
  }

  Box(Box&& other) noexcept : _meta{mem::take(other._meta)}, _data{mem::take(other._data)} {}

  Box& operator=(Box&& other) noexcept {
    if (this != &other) {
      if (_data) {
        (_meta->_dtor)(_data);
      }
      _meta = mem::take(other._meta);
      _data = mem::take(other._data);
    }
    return *this;
  }

  template <class X>
  static auto xnew(X x) noexcept -> Box {
    static const auto meta = Meta{
        [](void* p) { delete static_cast<X*>(p); },
        [](void* p, T&&... t) { return (*static_cast<X*>(p))(static_cast<T&&>(t)...); },
    };

    auto res = Box{};
    res._meta = &meta;
    res._data = new auto{mem::move(x)};
    return res;
  }

  explicit operator bool() const noexcept {
    return _meta != nullptr;
  }

  auto operator()(T... args) -> R {
    panicking::expect(_meta != nullptr, "boxed::Box::*: deref null");
    return (_meta->_call)(_data, static_cast<T&&>(args)...);
  }
};

template <class B>
auto box(B&& b) -> Box<B> {
  return Box<B>::xnew(static_cast<B&&>(b));
}

}  // namespace sfc::boxed

namespace sfc::option {
template <class... T>
class Inner<boxed::Box<T...>> {
  using Box = boxed::Box<T...>;
  Box _val{};

 public:
  Inner() noexcept = default;
  explicit Inner(Box&& val) noexcept : _val{static_cast<Box&&>(val)} {}
  ~Inner() noexcept = default;

  Inner(Inner&&) noexcept = default;

  auto is_some() const noexcept -> bool {
    return bool(_val);
  }

  auto is_none() const noexcept -> bool {
    return !bool(_val);
  }

  auto operator*() const noexcept -> const Box& {
    return _val;
  }

  auto operator*() noexcept -> Box& {
    return _val;
  }
};

}  // namespace sfc::option

namespace sfc {
using boxed::Box;
using boxed::box;
}  // namespace sfc
