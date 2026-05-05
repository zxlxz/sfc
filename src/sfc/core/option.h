#pragma once

#include "sfc/core/ops.h"
#include "sfc/core/ptr.h"
#include "sfc/core/panic.h"

namespace sfc::option {

template <class T>
concept Nullable = requires(const T& x) { x == nullptr; };

struct None {
  void fmt(auto& f) const {
    f.write_str("None()");
  }
};

template <class T>
class Option {
  bool _tag;
  union {
    T _val;
  };

 public:
  constexpr Option() noexcept : _tag{false} {}
  constexpr Option(None) noexcept : _tag{false} {}
  constexpr Option(T val) noexcept : _tag{true}, _val{static_cast<T&&>(val)} {}

  constexpr Option(Option&& other) noexcept : _tag{other._tag} {
    if (!_tag) return;
    ptr::write(&_val, static_cast<T&&>(other._val));
  }

  constexpr Option(const Option& other) noexcept : _tag{other._tag} {
    if (!_tag) return;
    ptr::write(&_val, other._val);
  }

  ~Option() noexcept {
    if (!_tag) return;
    _val.~T();
  }

  Option& operator=(const Option& other) noexcept {
    if (this == &other) return *this;
    if (_tag) {
      _val.~T();
      _tag = false;
    }
    if (other._tag) {
      ptr::write(&_val, other._val);
      _tag = other._tag;
    }
    return *this;
  }

  Option& operator=(Option&& other) noexcept {
    if (this == &other) return *this;
    if (_tag) {
      _val.~T();
      _tag = false;
    }
    if (other._tag) {
      ptr::write(&_val, static_cast<T&&>(other._val));
      _tag = other._tag;
    }
    return *this;
  }

  constexpr auto is_some() const noexcept -> bool {
    return _tag;
  }

  constexpr auto is_none() const noexcept -> bool {
    return !_tag;
  }

  constexpr explicit operator bool() const noexcept {
    return _tag;
  }

  auto operator->() const -> const T* {
    sfc::expect(_tag, "Option::operator->: not Some()");
    return &_val;
  }

  auto operator->() -> T* {
    sfc::expect(_tag, "Option::operator->: not Some()");
    return &_val;
  }

  auto operator*() const -> const T& {
    sfc::expect(_tag, "Option::operator*: not Some()");
    return _val;
  }

  auto operator*() -> T& {
    sfc::expect(_tag, "Option::operator*: not Some()");
    return _val;
  }

  auto expect(const auto& msg) && -> T {
    sfc::expect(_tag, "Option::expect: {}", msg);
    return static_cast<T&&>(_val);
  }

  auto unwrap() && -> T {
    sfc::expect(_tag, "Option::unwrap: not Some()");
    return static_cast<T&&>(_val);
  }

  auto unwrap_or(T default_val) && -> T {
    return _tag ? static_cast<T&&>(_val) : static_cast<T&&>(default_val);
  }

  template <class U>
  auto operator&(Option<U> optb) && -> Option<U> {
    if (_tag) return static_cast<Option<U>&&>(optb);
    return {};
  }

  auto operator|(Option<T> optb) && -> Option<T> {
    if (_tag) return static_cast<Option<T>&&>(*this);
    return static_cast<Option<T>&&>(optb);
  }

  template <class F>
  auto and_then(F&& op) && -> ops::invoke_t<F(T)> {
    if (_tag) return op(static_cast<T&&>(_val));
    return {};
  }

  auto or_else(auto&& f) && -> Option<T> {
    if (_tag) return static_cast<Option<T>&&>(*this);
    return f();
  }

  template <class F>
  auto map(F&& f) && -> Option<ops::invoke_t<F(T)>> {
    if (_tag) return f(static_cast<T&&>(_val));
    return {};
  }

  template <class U>
  auto map_or(U default_val, auto&& f) && -> U {
    if (_tag) return f(static_cast<T&&>(_val));
    return static_cast<U&&>(default_val);
  }

 public:
  // to result
  template <class E>
  auto ok_or(E err) && -> result::Result<T, E> {
    if (_tag) return static_cast<T&&>(_val);
    return static_cast<E&&>(err);
  }

 public:
  // trait: fmt::Display
  void fmt(auto& f) const {
    if (!_tag) {
      f.write_str("None()");
    } else {
      f.write_fmt("Some({})", _val);
    }
  }
};

template <Nullable T>
class Option<T> {
  T _inn;

 public:
  constexpr Option() noexcept : _inn{} {}
  constexpr Option(None) noexcept : _inn{} {}
  constexpr Option(T val) noexcept : _inn{static_cast<T&&>(val)} {}

  constexpr auto is_some() const noexcept -> bool {
    return _inn != nullptr;
  }

  constexpr auto is_none() const noexcept -> bool {
    return _inn == nullptr;
  }

  constexpr explicit operator bool() const noexcept {
    return _inn != nullptr;
  }

  auto operator->() const -> const T* {
    sfc::expect(_inn != nullptr, "Option::operator->: None()");
    return &_inn;
  }

  auto operator->() -> T* {
    sfc::expect(_inn != nullptr, "Option::operator->: None()");
    return &_inn;
  }

  auto operator*() const -> const T& {
    sfc::expect(_inn != nullptr, "Option::operator*: None()");
    return _inn;
  }

  auto operator*() -> T& {
    sfc::expect(_inn != nullptr, "Option::operator*: None()");
    return _inn;
  }

  auto expect(const auto& msg) && -> T {
    sfc::expect(_inn != nullptr, "Option::expect: {}", msg);
    return static_cast<T&&>(_inn);
  }

  auto unwrap() -> T {
    sfc::expect(_inn != nullptr, "Option::unwrap: None()");
    return static_cast<T&&>(_inn);
  }

  auto unwrap_or(T default_val) -> T {
    return _inn != nullptr ? static_cast<T&&>(_inn) : static_cast<T&&>(default_val);
  }

  template <class U>
  auto operator&(Option<U> optb) && -> Option<U> {
    if (_inn == nullptr) return {};
    return static_cast<Option<U>&&>(optb);
  }

  auto operator|(Option<T> optb) && -> Option<T> {
    if (_inn != nullptr) return static_cast<Option<T>&&>(*this);
    return static_cast<Option<T>&&>(optb);
  }

  template <class F>
  auto and_then(F&& op) && -> ops::invoke_t<F(T)> {
    if (_inn != nullptr) return op(static_cast<T&&>(_inn));
    return {};
  }

  auto or_else(auto&& f) && -> Option<T> {
    if (_inn != nullptr) return static_cast<Option<T>&&>(*this);
    return f();
  }

  template <class F>
  auto map(F&& f) && -> Option<ops::invoke_t<F(T)>> {
    if (_inn != nullptr) return {f(static_cast<T&&>(_inn))};
    return {};
  }

  template <class U>
  auto map_or(U default_val, auto&& f) && -> U {
    if (_inn != nullptr) return f(static_cast<T&&>(_inn));
    return static_cast<U&&>(default_val);
  }

 public:
  // to result
  template <class E>
  auto ok_or(E err) && noexcept -> result::Result<T, E> {
    if (_inn != nullptr) return {static_cast<T&&>(_inn)};
    return {static_cast<E&&>(err)};
  }

 public:
  // trait: fmt::Display
  void fmt(auto& f) const {
    if (_inn == nullptr) {
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
  constexpr Option(None) noexcept : _ptr{nullptr} {}
  constexpr Option(T& val) noexcept : _ptr{&val} {}

  constexpr auto is_some() const noexcept -> bool {
    return _ptr != nullptr;
  }

  constexpr auto is_none() const noexcept -> bool {
    return _ptr == nullptr;
  }

  constexpr explicit operator bool() const noexcept {
    return _ptr != nullptr;
  }

  auto operator*() const -> const T& {
    sfc::expect(_ptr != nullptr, "Option::deref: nullptr");
    return *_ptr;
  }

  auto operator*() -> T& {
    sfc::expect(_ptr != nullptr, "Option::deref: nullptr");
    return *_ptr;
  }

  auto unwrap() const -> T& {
    sfc::expect(_ptr != nullptr, "Option::unwrap: nullptr");
    return *_ptr;
  }

  auto unwrap_or(T& default_val) -> T& {
    return _ptr ? *_ptr : default_val;
  }

  auto expect(const auto& msg) const -> T& {
    sfc::expect(_ptr != nullptr, "Option::expect: {}", msg);
    return *_ptr;
  }

  auto to_owned() const -> Option<T> {
    if (!_ptr) return {};
    return {*_ptr};
  }

  template <class U>
  auto operator&(Option<U> optb) const -> Option<U> {
    if (_ptr) return static_cast<Option<U>&&>(optb);
    return {};
  }

  auto operator|(Option optb) const -> Option {
    if (_ptr) return *this;
    return optb;
  }

  template <class F>
  auto and_then(F&& op) -> ops::invoke_t<F(T&)> {
    if (_ptr) return op(*_ptr);
    return {};
  }

  auto or_else(auto&& f) -> Option {
    if (_ptr) return *this;
    return f();
  }

  template <class F>
  auto map(F&& f) -> Option<ops::invoke_t<F(T&)>> {
    if (_ptr) return {f(*_ptr)};
    return {};
  }

  template <class U>
  auto map_or(U default_val, auto&& f) -> U {
    if (_ptr) return f(*_ptr);
    return static_cast<U&&>(default_val);
  }

 public:
  // to result
  template <class E>
  auto ok_or(E err) -> result::Result<T&, E> {
    if (_ptr) return {*_ptr};
    return {static_cast<E&&>(err)};
  }

 public:
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
class Option<const T&> {
  const T* _ptr;

 public:
  constexpr Option() noexcept : _ptr{nullptr} {}
  constexpr Option(None) noexcept : _ptr{nullptr} {}
  constexpr Option(const T& val) noexcept : _ptr{&val} {}

  constexpr auto is_some() const noexcept -> bool {
    return _ptr != nullptr;
  }

  constexpr auto is_none() const noexcept -> bool {
    return _ptr == nullptr;
  }

  constexpr explicit operator bool() const noexcept {
    return _ptr != nullptr;
  }

  auto operator*() const -> const T& {
    sfc::expect(_ptr != nullptr, "Option::deref: nullptr");
    return *_ptr;
  }

  auto unwrap() const -> const T& {
    sfc::expect(_ptr != nullptr, "Option::unwrap: nullptr");
    return *_ptr;
  }

  auto expect(const auto& msg) const -> const T& {
    sfc::expect(_ptr != nullptr, "Option::expect: {}", msg);
    return *_ptr;
  }

  auto to_owned() const -> Option<T> {
    if (!_ptr) return {};
    return {*_ptr};
  }

  template <class U>
  auto operator&(Option<U> optb) const -> Option<U> {
    if (_ptr) return static_cast<Option<U>&&>(optb);
    return {};
  }

  auto operator|(Option<const T&> optb) const -> Option<const T&> {
    if (_ptr) return *this;
    return optb;
  }

  template <class F>
  auto and_then(F&& op) const -> ops::invoke_t<F(const T&)> {
    if (_ptr) return op(*_ptr);
    return {};
  }

  auto or_else(auto&& f) const -> Option<const T&> {
    if (_ptr) return *this;
    return f();
  }

  template <class F>
  auto map(F&& f) const -> Option<ops::invoke_t<F(const T&)>> {
    if (_ptr) return {f(*_ptr)};
    return {};
  }

  template <class U>
  auto map_or(U default_val, auto&& f) const -> U {
    if (_ptr) return f(*_ptr);
    return static_cast<U&&>(default_val);
  }

 public:
  // to result
  template <class E>
  auto ok_or(E err) const -> result::Result<const T&, E> {
    if (_ptr) return {*_ptr};
    return {static_cast<E&&>(err)};
  }

 public:
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

Option(const char*) -> Option<str::Str>;

template <class T>
auto operator==(const Option<T>& a, None) -> bool {
  return !a;
}

template <class A, class B>
auto operator==(const Option<A>& a, const Option<B>& b) -> bool {
  if (!a) return !b;
  if (!b) return false;
  return *a == *b;
}

}  // namespace sfc::option

namespace sfc {
using option::None;
using option::Option;
}  // namespace sfc
