#pragma once

#include "sfc/core/panicking.h"
#include "sfc/core/trait.h"

namespace sfc::option {

template <class T>
struct Inner {
  bool _tag = false;
  union {
    T _val;
  };

 public:
  [[gnu::always_inline]] explicit Inner() noexcept {}

  [[gnu::always_inline]] explicit Inner(T&& val) noexcept : _tag{true}, _val{static_cast<T&&>(val)} {}

  [[gnu::always_inline]] ~Inner() noexcept {
    if (_tag) {
      _val.~T();
    }
  }

  [[gnu::always_inline]] Inner(const Inner& other) noexcept : _tag{other._tag} {
    if (_tag) {
      new (&_val) T{other._val};
    }
  }

  [[gnu::always_inline]] Inner(Inner&& other) noexcept : _tag{other._tag} {
    if (_tag) {
      new (&_val) T{static_cast<T&&>(other._val)};
    }
  }

  [[gnu::always_inline]] Inner& operator=(const Inner& other) noexcept {
    if (this == &other) {
      return *this;
    }

    _tag ? _val.~T() : void();
    _tag = other._tag;
    _tag ? new (&_val) T{other._val} : void();
    return *this;
  }

  [[gnu::always_inline]] Inner& operator=(Inner&& other) noexcept {
    if (this == &other) {
      return *this;
    }
    _tag ? _val.~T() : void();
    _tag = other._tag;
    _tag ? new (&_val) T{static_cast<T&&>(other._val)} : void();
    return *this;
  }

  [[gnu::always_inline]] auto is_some() const noexcept -> bool {
    return _tag;
  }

  [[gnu::always_inline]] auto is_none() const noexcept -> bool {
    return !_tag;
  }

  [[gnu::always_inline]] auto operator*() const noexcept -> const T& {
    return _val;
  }

  [[gnu::always_inline]] auto operator*() noexcept -> T& {
    return _val;
  }
};

template <trait::tv_copy T>
struct Inner<T> {
  bool _tag = false;
  T _val;

 public:
  [[gnu::always_inline]] explicit Inner() noexcept : _val{} {}

  [[gnu::always_inline]] explicit Inner(T&& val) noexcept : _tag{true}, _val{static_cast<T&&>(val)} {}

  [[gnu::always_inline]] auto is_some() const noexcept -> bool {
    return _tag;
  }

  [[gnu::always_inline]] auto is_none() const noexcept -> bool {
    return !_tag;
  }

  [[gnu::always_inline]] auto operator*() const noexcept -> const T& {
    return _val;
  }

  [[gnu::always_inline]] auto operator*() noexcept -> T& {
    return _val;
  }
};

template <class T>
struct Inner<T&> {
  T* _ptr{nullptr};

 public:
  [[gnu::always_inline]] Inner() noexcept = default;

  [[gnu::always_inline]] Inner(T& val) noexcept : _ptr{&val} {}

  [[gnu::always_inline]] auto is_some() const noexcept -> bool {
    return _ptr != nullptr;
  }

  [[gnu::always_inline]] auto is_none() const noexcept -> bool {
    return _ptr == nullptr;
  }

  [[gnu::always_inline]] auto operator*() const noexcept -> const T& {
    return *_ptr;
  }

  [[gnu::always_inline]] auto operator*() noexcept -> T& {
    return *_ptr;
  }
};

template <class T>
class Option {
  using Some = T;
  using Inn = Inner<T>;

  Inn _inn{};

 public:
  Option() noexcept = default;
  Option(T val) noexcept : _inn{static_cast<T&&>(val)} {}
  ~Option() noexcept = default;

  template <class U>
  auto operator==(const Option<U>& other) const -> bool {
    if (this->is_none()) {
      return other.is_none();
    }
    return other.is_some() && *_inn == *other;
  }

  explicit operator bool() const noexcept {
    return _inn.is_some();
  }

  auto operator*() const -> const T& {
    panicking::expect(_inn.is_some(), "Option::operator*: not Some()");
    return *_inn;
  }

  auto operator*() -> T& {
    panicking::expect(_inn.is_some(), "Option::operator*: not Some()");
    return *_inn;
  }

  auto operator->() const {
    panicking::expect(_inn.is_some(), "Option::operator->: not Some()");
    return &*_inn;
  }

  auto operator->() {
    panicking::expect(_inn.is_some(), "Option::operator->: not Some()");
    return &*_inn;
  }

  auto is_some() const noexcept -> bool {
    return _inn.is_some();
  }

  auto is_none() const noexcept -> bool {
    return _inn.is_none();
  }

  auto unwrap() && -> T {
    panicking::expect(_inn.is_some(), "Option::unwrap: not Some()");
    return static_cast<T&&>(*_inn);
  }

  T unwrap_or(T default_val) && noexcept {
    if (_inn.is_some()) {
      return static_cast<T&&>(*_inn);
    }
    return static_cast<T&&>(default_val);
  }

  auto expect(const auto& msg) && -> T {
    panicking::expect(_inn.is_some(), "Option::expect: {}", msg);
    return static_cast<T&&>(*_inn);
  }

  template <class U>
  auto operator and(Option<U> optb) && -> Option<U> {
    if (_inn.is_some()) {
      return static_cast<Option<U>&&>(optb);
    }
    return {};
  }

  auto operator or(Option<T> optb) && -> Option<T> {
    if (_inn.is_some()) {
      return static_cast<Option&&>(*this);
    }
    return static_cast<Option&&>(optb);
  }

  template <class F>
  auto and_then(F&& op) && -> trait::invoke_t<F(T)> {
    if (_inn.is_none()) {
      return {};
    }
    return op(static_cast<T&&>(*_inn));
  }

  auto or_else(auto&& f) && -> Option<T> {
    if (_inn.is_some()) {
      return static_cast<T&&>(*_inn);
    }
    return f();
  }

  template <class F>
  auto map(F&& f) && -> Option<trait::invoke_t<F(T)>> {
    if (_inn.is_none()) {
      return {};
    }
    return {f(static_cast<T&&>(*_inn))};
  }

  template <class U>
  auto map_or(U default_val, auto&& f) && -> U {
    if (_inn.is_none()) {
      return static_cast<U&&>(default_val);
    }
    return f(static_cast<T&&>(*_inn));
  }

 public:
  // trait: fmt::Display
  void fmt(auto& f) const {
    if (_inn.is_none()) {
      f.write_str("None()");
    } else {
      f.write_fmt("Some({})", *_inn);
    }
  }
};

template <class T>
Option(T) -> Option<T>;

}  // namespace sfc::option

namespace sfc {
using option::Option;
}  // namespace sfc
