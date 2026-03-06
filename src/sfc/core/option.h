#pragma once

#include "sfc/core/expect.h"
#include "sfc/core/ops.h"
#include "sfc/core/ptr.h"

namespace sfc::option {

struct none_t {};
struct some_t {};

template <class T>
concept bool_ = requires {
  bool(T{});
  &T::operator bool;
};

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
      ptr::write(&_val, static_cast<T&&>(other._val));
    }
  }

  constexpr Inner& operator=(Inner&& other) noexcept {
    if (this != &other) {
      if (_tag) _val.~T();
      if (other._tag) ptr::write(&_val, static_cast<T&&>(other._val));
      _tag = other._tag;
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
    sfc::expect(bool(_inn), "Option::unwrap: not Some()");
    return static_cast<T&&>(*_inn);
  }

  auto unwrap_or(this auto&& self, T default_val) noexcept -> T {
    if (self._inn) {
      return static_cast<T&&>(*self._inn);
    }
    return static_cast<T&&>(default_val);
  }

  auto unwrap_unchecked() noexcept -> T {
    return static_cast<T&&>(*_inn);
  }

  auto expect(this auto&& self, const auto& msg) noexcept -> T {
    sfc::expect(bool(self._inn), "Option::expect: {}", msg);
    return static_cast<T&&>(*self._inn);
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
    } else {
      return other && (*_inn == *other);
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

template <bool_ T>
class Option<T> {
  T _inn;

 public:
  constexpr Option() noexcept : _inn{} {}
  constexpr Option(T val) noexcept : _inn{static_cast<T&&>(val)} {}

  constexpr Option(none_t) noexcept : _inn{} {}
  constexpr Option(some_t, auto&&... args) noexcept : _inn{static_cast<decltype(args)&&>(args)...} {}

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
    return &_inn;
  }

  auto operator->() noexcept {
    return &_inn;
  }

  auto operator*() const noexcept -> const T& {
    return _inn;
  }

  auto operator*() noexcept -> T& {
    return _inn;
  }

  auto unwrap(this auto self) noexcept -> T {
    sfc::expect(bool(self._inn), "Option::unwrap: not Some()");
    return static_cast<T&&>(self._inn);
  }

  auto unwrap_or(this auto self, T default_val) noexcept -> T {
    if (self._inn) {
      return static_cast<T&&>(self._inn);
    }
    return static_cast<T&&>(default_val);
  }

  auto unwrap_unchecked(this auto self) noexcept -> T {
    return static_cast<T&&>(self._inn);
  }

  auto expect(this auto self, const auto& msg) noexcept -> T {
    sfc::expect(bool(self._inn), "Option::expect: {}", msg);
    return static_cast<T&&>(self._inn);
  }

  template <class U>
  auto operator&(this auto self, Option<U> optb) noexcept -> Option<U> {
    if (self._inn) {
      return static_cast<Option<U>&&>(optb);
    }
    return {};
  }

  auto operator|(this auto self, Option<T> optb) noexcept -> Option<T> {
    if (self._inn) {
      return static_cast<Option<T>&&>(self);
    }
    return static_cast<Option<T>&&>(optb);
  }

  template <class F>
  auto and_then(this auto self, F&& op) noexcept -> ops::invoke_t<F(T)> {
    if (!self._inn) {
      return {};
    }
    return op(static_cast<T&&>(self._inn));
  }

  auto or_else(this auto self, auto&& f) noexcept -> Option<T> {
    if (self._inn) {
      return static_cast<T&&>(self._inn);
    }
    return f();
  }

  template <class F>
  auto map(this auto self, F&& f) -> Option<ops::invoke_t<F(T)>> {
    if (!self._inn) {
      return {};
    }
    return {f(static_cast<T&&>(self._inn))};
  }

  template <class U>
  auto map_or(this auto self, U default_val, auto&& f) -> U {
    if (!self._inn) {
      return static_cast<U&&>(default_val);
    }
    return f(static_cast<T&&>(self._inn));
  }

 public:
  // to result
  template <class E>
  auto ok_or(this auto self, E err) -> result::Result<T, E> {
    if (self._inn) {
      return {static_cast<T&&>(self._inn)};
    }
    return {static_cast<E&&>(err)};
  }

 public:
  // trait: ops::Eq
  template <class U>
  auto operator==(const Option<U>& other) const -> bool {
    if (!_inn) {
      return !other;
    } else {
      return other && (_inn == *other);
    }
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    if (!_inn) {
      f.write_str("None()");
    } else {
      f.write_fmt("Some({})", _inn);
    }
  }
};

template <class T>
class Option<T&> {
  T* _ptr;

 public:
  constexpr Option() noexcept : _ptr{nullptr} {}
  constexpr Option(T& val) noexcept : _ptr{&val} {}

  constexpr Option(none_t) noexcept : _ptr{nullptr} {}
  constexpr Option(some_t, T& val) noexcept : _ptr{&val} {}

  constexpr auto is_some() const noexcept -> bool {
    return _ptr != nullptr;
  }

  constexpr auto is_none() const noexcept -> bool {
    return _ptr == nullptr;
  }

  constexpr explicit operator bool() const noexcept {
    return _ptr != nullptr;
  }

  auto operator->() const {
    sfc::expect(_ptr != nullptr, "Option<T&>::deref: nullptr");
    return _ptr;
  }

  auto operator->() {
    sfc::expect(_ptr != nullptr, "Option<T&>::deref: nullptr");
    return _ptr;
  }

  auto operator*() const -> const T& {
    sfc::expect(_ptr != nullptr, "Option<T&>::deref: nullptr");
    return *_ptr;
  }

  auto operator*() -> T& {
    sfc::expect(_ptr != nullptr, "Option<T&>::deref: nullptr");
    return *_ptr;
  }

  auto unwrap() const -> T& {
    sfc::expect(_ptr != nullptr, "Option<T&>::unwrap: nullptr");
    return *_ptr;
  }

  auto unwrap_or(T& default_val) noexcept -> T& {
    return _ptr ? *_ptr : default_val;
  }

  auto unwrap_unchecked() noexcept -> T& {
    return *_ptr;
  }

  auto expect(const auto& msg) const -> T& {
    sfc::expect(_ptr != nullptr, "Option::expect: {}", msg);
    return *_ptr;
  }

  template <class U>
  auto operator&(Option<U> optb) const noexcept -> Option<U> {
    return _ptr ? mem::move(optb) : Option<U>{};
  }

  auto operator|(Option<T> optb) const noexcept -> Option<T> {
    return _ptr ? *this : optb;
  }

  template <class F>
  auto and_then(F&& op) && noexcept -> ops::invoke_t<F(T)> {
    return _ptr ? op(*_ptr) : ops::invoke_t<F(T)>{};
  }

  auto or_else(auto&& f) && noexcept -> Option<T> {
    return _ptr ? *_ptr : f();
  }

  template <class F>
  auto map(F&& f) -> Option<ops::invoke_t<F(T)>> {
    using U = ops::invoke_t<F(T)>;
    return _ptr ? Option<U>{f(*_ptr)} : Option<U>{};
  }

  template <class U>
  auto map_or(U default_val, auto&& f) -> U {
    return _ptr ? f(*_ptr) : static_cast<U&&>(default_val);
  }

 public:
  // to result
  template <class E>
  auto ok_or(E err) && -> result::Result<T, E>;

 public:
  // trait: ops::Eq
  template <class U>
  auto operator==(const Option<U>& other) const -> bool {
    if (!_ptr) {
      return !other;
    } else {
      return other && (*_ptr == *other);
    }
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    if (!_ptr) {
      f.write_str("None()");
    } else {
      f.write_fmt("Some({})", *_ptr);
    }
  }
};

template <class T>
Option(T) -> Option<T>;

}  // namespace sfc::option

namespace sfc {
using option::Option;
}  // namespace sfc
