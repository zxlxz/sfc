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

  Box(Box&& other) noexcept : _ptr{other._ptr}, _alloc{static_cast<A&&>(other._alloc)} {
    other._ptr = {};
  }

  auto operator=(Box&& other) noexcept -> Box& {
    if (this == &other) {
      return *this;
    }
    this->reset();
    _ptr = other._ptr, other._ptr = {};
    _alloc = static_cast<A&&>(other._alloc);
    return *this;
  }

  static auto from_raw(T* ptr, A alloc = {}) -> Box {
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

  void reset() noexcept {
    if (_ptr) {
      _ptr->~T();
      _alloc.dealloc(_ptr, Layout::of<T>());
      _ptr = nullptr;
    }
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
  union Data {
    void* _ptr = nullptr;  // pointer to heap-allocated object
    usize _buf[1];         // SSO buffer for small and trivial objects
  };

  struct Meta {
    Layout _layout = {};
    void (*_dtor)(Data&) = nullptr;  // only non-SSO have dtor
    R (*_call)(Data&, T&&...) = nullptr;
  };

  template <class X>
  struct Impl {
    using U = Data;
    static constexpr auto SSO = __is_trivially_copyable(X) && sizeof(X) <= sizeof(void*);

    static auto meta() -> Meta {
      const auto dtor = SSO ? nullptr : +[](U& u) { static_cast<X*>(u._ptr)->~X(); };
      const auto call =
          SSO ? [](U& u, T&&... t) { return (reinterpret_cast<X&>(u))(static_cast<T&&>(t)...); }
              : [](U& u, T&&... t) { return (*static_cast<X*>(u._ptr))(static_cast<T&&>(t)...); };

      return Meta{Layout::of<X>(), dtor, call};
    }

    static auto xnew(X&& x, auto& alloc) -> Data {
      auto res = Data{};
      res._ptr = SSO ? res._buf : alloc.alloc(Layout::of<X>());
      new (res._ptr) X{static_cast<X&&>(x)};
      return res;
    }
  };

  const Meta* _meta{nullptr};
  Data _data{};
  [[no_unique_address]] A _alloc{};

 public:
  Box() noexcept = default;

  ~Box() {
    if (_meta && _meta->_dtor) {
      // only non-SSO will have dtor
      (_meta->_dtor)(_data), _alloc.dealloc(_data._ptr, _meta->_layout);
    }
  }

  Box(Box&& other) noexcept
      : _meta{other._meta}, _data{other._data}, _alloc{static_cast<A&&>(other._alloc)} {
    other._meta = {};
    other._data = {};
  }

  Box& operator=(Box&& other) noexcept {
    if (this == &other) {
      return *this;
    }
    if (_meta && _meta->_dtor) {
      (_meta->_dtor)(_data);
      _alloc.dealloc(_data._ptr, _meta->_layout);
    }
    _meta = other._meta, other._meta = {};
    _data = other._data, other._data = {};
    _alloc = static_cast<A&&>(other._alloc);
    return *this;
  }

  template <class X>
  static auto xnew(X fun) -> Box {
    using Impl = Box::Impl<X>;
    static const auto meta = Impl::meta();

    auto res = Box{};
    res._meta = &meta;
    res._data = Impl::xnew(static_cast<X&&>(fun), res._alloc);
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

template <class T, class A>
class Box<T&, A> {
  static_assert(__is_class(T));
  friend T;

  struct Meta {
    Layout _layout = {};
    void (*_dtor)(void*) = nullptr;

    template <class X>
    Meta(X*) : _layout(Layout::of<X>()), _dtor([](void* p) { static_cast<X*>(p)->~X(); }) {}
  };

  const Meta* _meta = nullptr;
  void* _self = nullptr;
  [[no_unique_address]] A _alloc = {};

 public:
  Box() noexcept = default;

  ~Box() {
    this->reset();
  }

  Box(Box&& other) noexcept
      : _meta{other._meta}, _self{other._self}, _alloc{static_cast<A&&>(other._alloc)} {
    other._meta = {};
    other._self = {};
  }

  auto operator=(Box&& other) noexcept -> Box& {
    if (this == &other) {
      return *this;
    }
    this->reset();
    _meta = other._meta, other._meta = {};
    _self = other._self, other._self = {};
    return *this;
  }

  template <class X>
  static auto xnew(X x) -> Box {
    static const auto meta = typename T::Meta{&x};

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
    panicking::assert(_self != nullptr, "boxed::Box::->: deref null");
    return reinterpret_cast<T*>(this);
  }

  void reset() noexcept {
    if (_self) {
      (_meta->_dtor)(_self);
      _alloc.dealloc(_self, _meta->_layout);
      _self = nullptr;
    }
  }
};

}  // namespace sfc::boxed

namespace sfc::option::detail {

template <class... T>
class Option<boxed::Box<T...>> {
  using Box = boxed::Box<T...>;
  Box _val{};

 public:
  Option() = default;
  explicit Option(Box&& val) noexcept : _val{static_cast<Box&&>(val)} {}

  Option(Option&&) noexcept = default;
  Option& operator=(Option&&) noexcept = default;

  auto tag() const noexcept -> Tag {
    return _val ? Tag::Some : Tag::None;
  }

  auto operator*() const noexcept -> const Box& {
    return _val;
  }

  auto operator*() noexcept -> Box& {
    return _val;
  }
};

}  // namespace sfc::option::detail
