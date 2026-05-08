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
  u8 _tag;
  union {
    T _1;
  };

 public:
  constexpr Option() noexcept : _tag{0} {}
  constexpr Option(None) noexcept : _tag{0} {}
  constexpr Option(T val) noexcept : _tag{1}, _1{static_cast<T&&>(val)} {}

  constexpr Option(Option&& other) noexcept : _tag{0} {
    switch (other._tag) {
      case 0:  break;
      case 1:  ptr::write(&_1, static_cast<T&&>(other._1)), _tag = 1; break;
      default: break;
    }
  }

  constexpr Option(const Option& other) noexcept : _tag{0} {
    switch (other._tag) {
      case 0:  break;
      case 1:  ptr::write(&_1, other._1), _tag = 1; break;
      default: break;
    }
  }

  ~Option() noexcept {
    switch (_tag) {
      case 0:  break;
      case 1:  mem::drop(_1); break;
      default: break;
    }
  }

  Option& operator=(const Option& other) noexcept {
    if (this == &other) return *this;
    switch (_tag) {
      case 0:  break;
      case 1:  mem::drop(_1), _tag = 0; break;
      default: break;
    }
    switch (other._tag) {
      case 0:  break;
      case 1:  ptr::write(&_1, other._1), _tag = 1; break;
      default: break;
    }
    return *this;
  }

  Option& operator=(Option&& other) noexcept {
    if (this == &other) return *this;
    switch (_tag) {
      case 0:  break;
      case 1:  mem::drop(_1), _tag = 0; break;
      default: break;
    }
    switch (other._tag) {
      case 0:  break;
      case 1:  ptr::write(&_1, static_cast<T&&>(other._1)), _tag = 1; break;
      default: break;
    }
    return *this;
  }

  constexpr auto is_some() const noexcept -> bool {
    return _tag == 1;
  }

  constexpr auto is_none() const noexcept -> bool {
    return _tag == 0;
  }

  constexpr explicit operator bool() const noexcept {
    return _tag == 1;
  }

  auto operator->() const -> const T* {
    sfc::expect(_tag == 1, "Option::operator->: not Some()");
    return &_1;
  }

  auto operator->() -> T* {
    sfc::expect(_tag == 1, "Option::operator->: not Some()");
    return &_1;
  }

  auto operator*() const -> const T& {
    sfc::expect(_tag == 1, "Option::operator*: not Some()");
    return _1;
  }

  auto operator*() -> T& {
    sfc::expect(_tag == 1, "Option::operator*: not Some()");
    return _1;
  }

  auto expect(const auto& msg) && -> T {
    sfc::expect(_tag == 1, "Option::expect: {}", msg);
    return static_cast<T&&>(_1);
  }

  auto unwrap() && -> T {
    sfc::expect(_tag == 1, "Option::unwrap: not Some()");
    return static_cast<T&&>(_1);
  }

  auto unwrap_or(T default_val) && -> T {
    if (_tag == 1) return static_cast<T&&>(_1);
    return static_cast<T&&>(default_val);
  }

  template <class U>
  auto operator&(Option<U> optb) && -> Option<U> {
    if (_tag == 1) return static_cast<Option<U>&&>(optb);
    return None{};
  }

  auto operator|(Option<T> optb) && -> Option<T> {
    if (_tag == 1) return static_cast<Option<T>&&>(*this);
    return static_cast<Option<T>&&>(optb);
  }

  template <class F>
  auto and_then(F&& op) && -> ops::invoke_t<F(T)> {
    if (_tag == 1) return op(static_cast<T&&>(_1));
    return None{};
  }

  auto or_else(auto&& f) && -> Option<T> {
    if (_tag == 1) return static_cast<Option<T>&&>(*this);
    return f();
  }

  template <class F>
  auto map(F&& f) && -> Option<ops::invoke_t<F(T)>> {
    if (_tag == 1) return f(static_cast<T&&>(_1));
    return None{};
  }

  template <class U>
  auto map_or(U default_val, auto&& f) && -> U {
    if (_tag == 1) return f(static_cast<T&&>(_1));
    return static_cast<U&&>(default_val);
  }

 public:
  // to result
  template <class E>
  auto ok_or(E err) && -> result::Result<T, E> {
    if (_tag == 1) return static_cast<T&&>(_1);
    return static_cast<E&&>(err);
  }

 public:
  // trait: fmt::Display
  void fmt(auto& f) const {
    if (_tag == 0) {
      f.write_str("None()");
    } else {
      f.write_fmt("Some({})", _1);
    }
  }
};

template <Nullable T>
class Option<T> {
  T _1;

 public:
  constexpr Option() noexcept : _1{} {}
  constexpr Option(None) noexcept : _1{} {}
  constexpr Option(T val) noexcept : _1{static_cast<T&&>(val)} {}

  constexpr auto is_some() const noexcept -> bool {
    return _1 != nullptr;
  }

  constexpr auto is_none() const noexcept -> bool {
    return _1 == nullptr;
  }

  constexpr explicit operator bool() const noexcept {
    return _1 != nullptr;
  }

  auto operator->() const -> const T* {
    sfc::expect(_1 != nullptr, "Option::operator->: None()");
    return &_1;
  }

  auto operator->() -> T* {
    sfc::expect(_1 != nullptr, "Option::operator->: None()");
    return &_1;
  }

  auto operator*() const -> const T& {
    sfc::expect(_1 != nullptr, "Option::operator*: None()");
    return _1;
  }

  auto operator*() -> T& {
    sfc::expect(_1 != nullptr, "Option::operator*: None()");
    return _1;
  }

  auto expect(const auto& msg) && -> T {
    sfc::expect(_1 != nullptr, "Option::expect: {}", msg);
    return static_cast<T&&>(_1);
  }

  auto unwrap() -> T {
    sfc::expect(_1 != nullptr, "Option::unwrap: None()");
    return static_cast<T&&>(_1);
  }

  auto unwrap_or(T default_val) -> T {
    return _1 != nullptr ? static_cast<T&&>(_1) : static_cast<T&&>(default_val);
  }

  template <class U>
  auto operator&(Option<U> optb) && -> Option<U> {
    if (_1 != nullptr) return static_cast<Option<U>&&>(optb);
    return None{};
  }

  auto operator|(Option<T> optb) && -> Option<T> {
    if (_1 != nullptr) return static_cast<Option<T>&&>(*this);
    return static_cast<Option<T>&&>(optb);
  }

  template <class F>
  auto and_then(F&& op) && -> ops::invoke_t<F(T)> {
    if (_1 != nullptr) return op(static_cast<T&&>(_1));
    return None{};
  }

  auto or_else(auto&& f) && -> Option<T> {
    if (_1 != nullptr) return static_cast<Option<T>&&>(*this);
    return f();
  }

  template <class F>
  auto map(F&& f) && -> Option<ops::invoke_t<F(T)>> {
    if (_1 != nullptr) return {f(static_cast<T&&>(_1))};
    return {};
  }

  template <class U>
  auto map_or(U default_val, auto&& f) && -> U {
    if (_1 != nullptr) return f(static_cast<T&&>(_1));
    return static_cast<U&&>(default_val);
  }

 public:
  // to result
  template <class E>
  auto ok_or(E err) && noexcept -> result::Result<T, E> {
    if (_1 != nullptr) return {static_cast<T&&>(_1)};
    return {static_cast<E&&>(err)};
  }

 public:
  // trait: fmt::Display
  void fmt(auto& f) const {
    if (_1 == nullptr) {
      f.write_str("None()");
    } else {
      f.write_fmt("Some({})", _1);
    }
  }
};

template <class T>
class Option<T&> {
  T* _1;

 public:
  constexpr Option() noexcept : _1{nullptr} {}
  constexpr Option(None) noexcept : _1{nullptr} {}
  constexpr Option(T& val) noexcept : _1{&val} {}

  constexpr auto is_some() const noexcept -> bool {
    return _1 != nullptr;
  }

  constexpr auto is_none() const noexcept -> bool {
    return _1 == nullptr;
  }

  constexpr explicit operator bool() const noexcept {
    return _1 != nullptr;
  }

  auto operator->() const -> const T* {
    sfc::expect(_1 != nullptr, "Option::operator->: None()");
    return _1;
  }

  auto operator->() -> T* {
    sfc::expect(_1 != nullptr, "Option::operator->: None()");
    return _1;
  }

  auto operator*() const -> const T& {
    sfc::expect(_1 != nullptr, "Option::deref: nullptr");
    return *_1;
  }

  auto operator*() -> T& {
    sfc::expect(_1 != nullptr, "Option::deref: nullptr");
    return *_1;
  }

  auto unwrap() const -> T& {
    sfc::expect(_1 != nullptr, "Option::unwrap: nullptr");
    return *_1;
  }

  auto unwrap_or(T& default_val) -> T& {
    return _1 ? *_1 : default_val;
  }

  auto unwrap_or_else(auto&& f) -> T& {
    if (_1) return *_1;
    return f();
  }

  auto expect(const auto& msg) const -> T& {
    sfc::expect(_1 != nullptr, "Option::expect: {}", msg);
    return *_1;
  }

  auto to_owned() const -> Option<T> {
    if (!_1) return {};
    return {*_1};
  }

  template <class U>
  auto operator&(Option<U> optb) const -> Option<U> {
    if (_1) return static_cast<Option<U>&&>(optb);
    return None{};
  }

  auto operator|(Option optb) const -> Option {
    if (_1) return *this;
    return optb;
  }

  template <class F>
  auto and_then(F&& op) -> ops::invoke_t<F(T&)> {
    if (_1) return op(*_1);
    return None{};
  }

  auto or_else(auto&& f) -> Option<T&> {
    if (_1) return *this;
    return f();
  }

  template <class F>
  auto map(F&& f) -> Option<ops::invoke_t<F(T&)>> {
    if (_1) return {f(*_1)};
    return None{};
  }

  template <class U>
  auto map_or(U default_val, auto&& f) -> U {
    if (_1) return f(*_1);
    return static_cast<U&&>(default_val);
  }

 public:
  // to result
  template <class E>
  auto ok_or(E err) -> result::Result<T&, E> {
    if (_1) return {*_1};
    return {static_cast<E&&>(err)};
  }

 public:
  // trait: fmt::Display
  void fmt(auto& f) const {
    if (!_1) {
      f.write_str("None()");
    } else {
      f.write_fmt("Some({})", *_1);
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

  auto operator->() const -> const T* {
    sfc::expect(_ptr != nullptr, "Option::operator->: None()");
    return _ptr;
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
    return None{};
  }

  auto operator|(Option<const T&> optb) const -> Option<const T&> {
    if (_ptr) return *this;
    return optb;
  }

  template <class F>
  auto and_then(F&& op) const -> ops::invoke_t<F(const T&)> {
    if (_ptr) return op(*_ptr);
    return None{};
  }

  auto or_else(auto&& f) const -> Option<const T&> {
    if (_ptr) return *this;
    return f();
  }

  template <class F>
  auto map(F&& f) const -> Option<ops::invoke_t<F(const T&)>> {
    if (_ptr) return {f(*_ptr)};
    return None{};
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
