#pragma once

#include "sfc/core/panicking.h"
#include "sfc/core/ops.h"

namespace sfc::option {

struct none_t {};
struct some_t {};

template <class T>
struct Inner {
  bool _tag = false;
  union {
    T _val;
  };

 public:
  constexpr Inner(none_t) noexcept {}
  constexpr Inner(some_t, auto&&... args) noexcept : _tag{true}, _val{static_cast<decltype(args)&&>(args)...} {}

  constexpr ~Inner() noexcept {
    if (_tag) {
      _val.~T();
    }
  }

  constexpr Inner(Inner&& other) noexcept : _tag{other._tag} {
    if (_tag) {
      new (&_val) T{static_cast<T&&>(other._val)};
    }
  }

  constexpr Inner& operator=(Inner&& other) noexcept {
    if (this != &other) {
      _tag ? _val.~T() : void();
      _tag = other._tag;
      _tag ? new (&_val) T{static_cast<T&&>(other._val)} : &_val;
    }
    return *this;
  }

  constexpr explicit operator bool() const noexcept {
    return _tag;
  }

  constexpr auto operator*() const noexcept -> const T& {
    return _val;
  }

  constexpr auto operator*() noexcept -> T& {
    return _val;
  }
};

template <class T>
struct Inner<T&> {
  T* _ptr{nullptr};

 public:
  constexpr Inner(none_t) noexcept {}
  constexpr Inner(some_t, T& val) noexcept : _ptr{&val} {}

  constexpr explicit operator bool() const noexcept {
    return _ptr != nullptr;
  }

  constexpr auto operator*() const noexcept -> const T& {
    return *_ptr;
  }

  constexpr auto operator*() noexcept -> T& {
    return *_ptr;
  }
};

template <trait::tv_copy_ T>
struct Inner<T> {
  bool _tag = false;
  union {
    T _val;
  };

 public:
  constexpr Inner(none_t) noexcept {}

  constexpr Inner(some_t, auto&&... args) noexcept : _tag{true}, _val{static_cast<decltype(args)&&>(args)...} {}

  constexpr operator bool() const noexcept {
    return _tag;
  }

  constexpr auto operator*() const noexcept -> const T& {
    return _val;
  }

  constexpr auto operator*() noexcept -> T& {
    return _val;
  }
};

template <class T>
class Option {
  Inner<T> _inn{};

 public:
  constexpr Option() noexcept : _inn{none_t{}} {}
  constexpr Option(T val) noexcept : _inn{some_t{}, static_cast<T&&>(val)} {}

  constexpr Option(none_t) noexcept : _inn{none_t{}} {}
  constexpr Option(some_t, auto&&... args) noexcept : _inn{some_t{}, static_cast<decltype(args)&&>(args)...} {}

  constexpr auto is_some() const noexcept -> bool {
    return bool(_inn);
  }

  constexpr auto is_none() const noexcept -> bool {
    return !bool(_inn);
  }

  constexpr explicit operator bool() const noexcept {
    return bool(_inn);
  }

  auto operator->() const noexcept {
    return &*_inn;
  }

  auto operator->() noexcept {
    return &*_inn;
  }

  auto operator*() const noexcept -> const T& {
    return *_inn;
  }

  auto operator*() noexcept -> T& {
    return *_inn;
  }

  auto unwrap() && noexcept -> T {
    panicking::expect(bool(_inn), "Option::unwrap: not Some()");
    return static_cast<T&&>(*_inn);
  }

  auto unwrap_or(T default_val) && noexcept -> T {
    if (_inn) {
      return static_cast<T&&>(*_inn);
    }
    return static_cast<T&&>(default_val);
  }

  auto expect(const auto& msg) && noexcept -> T {
    panicking::expect(bool(_inn), "Option::expect: {}", msg);
    return static_cast<T&&>(*_inn);
  }

  template <class U>
  auto operator&(Option<U> optb) && noexcept -> Option<U> {
    if (_inn) {
      return static_cast<Option<U>&&>(optb);
    }
    return {};
  }

  auto operator|(Option<T> optb) && noexcept -> Option<T> {
    if (_inn) {
      return static_cast<Option&&>(*this);
    }
    return static_cast<Option&&>(optb);
  }

  template <class F>
  auto and_then(F&& op) && noexcept -> ops::invoke_t<F(T)> {
    if (!_inn) {
      return {};
    }
    return op(static_cast<T&&>(*_inn));
  }

  auto or_else(auto&& f) && noexcept -> Option<T> {
    if (_inn) {
      return static_cast<T&&>(*_inn);
    }
    return f();
  }

  template <class F>
  auto map(F&& f) -> Option<ops::invoke_t<F(T)>> {
    if (!_inn) {
      return {};
    }
    return {f(static_cast<T&&>(*_inn))};
  }

  template <class U>
  auto map_or(U default_val, auto&& f) -> U {
    if (!_inn) {
      return static_cast<U&&>(default_val);
    }
    return f(static_cast<T&&>(*_inn));
  }

 public:
  // to result
  template <class E>
  auto ok_or(E err) && -> result::Result<T, E>;

 public:
  // trait: ops::Eq
  template <class U>
  auto operator==(const Option<U>& other) const -> bool {
    if (!_inn) {
      return !other;
    } else if (other) {
      return (*_inn == *other);
    } else {
      return false;
    }
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    if (!_inn) {
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
