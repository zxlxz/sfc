#pragma once

#include "sfc/alloc/alloc.h"

namespace sfc::boxed {

using Layout = alloc::Layout;

template <class T, class A = alloc::Global>
class Box {
  T* _ptr = nullptr;
  [[no_unique_address]] A _alloc = {};

 public:
  Box() noexcept = default;

  ~Box() {
    this->reset();
  }

  Box(const Box&) = delete;
  Box& operator=(const Box&) = delete;

  Box(Box&& other) noexcept : _ptr{other._ptr}, _alloc{static_cast<A&&>(other._alloc)} {
    other._ptr = {};
  }

  auto operator=(Box&& other) noexcept -> Box& {
    if (this == &other) {
      return *this;
    }
    this->reset();
    _ptr = mem::take(other._ptr);
    _alloc = mem::move(other._alloc);
    return *this;
  }

  static auto from_raw(T* ptr, A alloc = {}) noexcept -> Box {
    auto res = Box{};
    res._ptr = ptr;
    res._alloc = static_cast<A&&>(alloc);
    return res;
  }

  static auto xnew(auto&&... args) -> Box {
    auto res = Box{};
    try {
      res._ptr = static_cast<T*>(res._alloc.alloc(Layout::of<T>()));
      new (res._ptr) T{static_cast<decltype(args)&&>(args)...};
    } catch (...) {
      res._alloc.dealloc(res._ptr, Layout::of<T>());
      res._ptr = {};
      throw;
    }
    return res;
  }

  explicit operator bool() const noexcept {
    return _ptr != nullptr;
  }

  auto ptr() const -> T* {
    return _ptr;
  }

  auto into_raw() && -> T* {
    const auto res = _ptr;
    _ptr = nullptr;
    return res;
  }

  auto operator->() const -> const T* {
    panicking::expect(_ptr != nullptr, "boxed::Box::->: deref null");
    return _ptr;
  }

  auto operator->() -> T* {
    panicking::expect(_ptr != nullptr, "boxed::Box::->: deref null");
    return _ptr;
  }

  auto operator*() const -> const T& {
    panicking::expect(_ptr != nullptr, "boxed::Box::*: deref null");
    return *_ptr;
  }

  auto operator*() -> T& {
    panicking::expect(_ptr != nullptr, "boxed::Box::*: deref null");
    return *_ptr;
  }

  void reset() noexcept {
    if (!_ptr) {
      return;
    }
    _ptr->~T();
    _alloc.dealloc(_ptr, Layout::of<T>());
    _ptr = nullptr;
  }

  void fmt(auto& f) const {
    if (_ptr == nullptr) {
      f.write_str("Box()");
    } else {
      f.write_fmt("Box({})", *_ptr);
    }
  }
};

template <class R, class... T, class A>
class Box<R(T...), A> {
  struct Meta {
    Layout _layout = {};
    void (*_dtor)(void*) = nullptr;  // only non-SSO have dtor
    R (*_call)(void*, T&&...) = nullptr;

    template <class X>
    static auto from(const X&) -> Meta {
      static constexpr auto SSO = __is_trivially_copyable(X) && sizeof(X) <= sizeof(void*);
      const auto dtor = [](void* p) { static_cast<X*>(p)->~X(); };
      const auto call = [](void* p, T&&... t) { return (*static_cast<X*>(p))(static_cast<T&&>(t)...); };
      return Meta{Layout::of<X>(), SSO ? nullptr : dtor, call};
    }

    void drop(void* p) {
      if (!_dtor) {
        return;
      }
      _dtor(p);
    }
  };

  const Meta* _meta{nullptr};
  void* _data{nullptr};
  [[no_unique_address]] A _alloc{};

 public:
  Box() noexcept = default;

  ~Box() {
    this->reset();
  }

  Box(const Box&) = delete;
  Box& operator=(const Box&) = delete;

  Box(Box&& other) noexcept
      : _meta{mem::take(other._meta)}, _data{mem::take(other._data)}, _alloc{static_cast<A&&>(other._alloc)} {}

  Box& operator=(Box&& other) noexcept {
    if (this == &other) {
      return *this;
    }
    this->reset();
    _meta = mem::take(other._meta);
    _data = mem::take(other._data);
    _alloc = mem::move(other._alloc);
    return *this;
  }

  template <class X>
  static auto xnew(X obj) -> Box {
    static const auto meta = Meta::from(obj);

    auto res = Box{};
    res._meta = &meta;
    if (meta._dtor) {
      res._data = res._alloc.alloc(meta._layout);
      new (res._data) auto{static_cast<X&&>(obj)};
    } else {
      new (&res._data) auto{static_cast<X&&>(obj)};
    }
    return res;
  }

  explicit operator bool() const {
    return _meta != nullptr;
  }

  auto operator()(T... args) -> auto {
    panicking::expect(_meta != nullptr, "boxed::Box::*: deref null");
    const auto ptr = _meta->_dtor ? _data : static_cast<void*>(&_data);
    return (_meta->_call)(ptr, static_cast<T&&>(args)...);
  }

  void reset() {
    if (_meta == nullptr) {
      return;
    }
    if (_meta->_dtor) {
      _meta->_dtor(_data);
      _alloc.dealloc(_data, _meta->_layout);
    }
    _data = {};
    _meta = nullptr;
  }
};

template <class T, class A>
class Box<T&, A> {
  static_assert(__is_class(T));

  struct Meta : T::Meta {
    Layout _layout = {};
    void (*_dtor)(void*) = nullptr;

    template <class X>
    static auto from(const X& _) -> Meta {
      const auto layout = Layout::of<X>();
      const auto dtor = [](void* p) { static_cast<X*>(p)->~X(); };
      return {T::Meta::from(_), layout, dtor};
    }
  };

  const Meta* _meta = nullptr;
  void* _self = nullptr;
  [[no_unique_address]] A _alloc = {};

 public:
  Box() noexcept = default;

  ~Box() noexcept {
    this->reset();
  }

  Box(const Box&) = delete;

  Box& operator=(const Box&) = delete;

  Box(Box&& other) noexcept
      : _meta{mem::take(other._meta)}, _self{mem::take(other._self)}, _alloc{mem::move(other._alloc)} {}

  Box& operator=(Box&& other) noexcept {
    if (this == &other) {
      return *this;
    }
    this->reset();
    _meta = mem::take(other._meta);
    _self = mem::take(other._self);
    _alloc = mem::move(other._alloc);
    return *this;
  }

  template <class X>
  static auto xnew(X x) -> Box {
    static const auto meta = Meta::from(x);

    auto res = Box{};
    res._meta = &meta;
    res._self = res._alloc.alloc(meta._layout);
    new (res._self) X{static_cast<X&&>(x)};
    return res;
  }

  explicit operator bool() const {
    return _self != nullptr;
  }

  auto operator->() -> T* {
    panicking::expect(_self != nullptr, "boxed::Box::->: deref null");
    return reinterpret_cast<T*>(this);
  }

  void reset() noexcept {
    if (!_self) {
      return;
    }
    (_meta->_dtor)(_self);
    _alloc.dealloc(_self, _meta->_layout);
    _self = nullptr;
  }
};

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
}  // namespace sfc
