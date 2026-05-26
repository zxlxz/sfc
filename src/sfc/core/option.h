#pragma once

#include "sfc/core/ops.h"
#include "sfc/core/ptr.h"
#include "sfc/core/panic.h"

namespace sfc::option {

template <class T>
concept null_ = requires(const T& x) { x == nullptr; };

template <class T = void>
class Option;

template <>
class Option<void> {
 public:
  constexpr auto is_some() const noexcept -> bool {
    return false;
  }

  constexpr auto is_none() const noexcept -> bool {
    return true;
  }

  constexpr explicit operator bool() const noexcept {
    return false;
  }

  void fmt(auto& f) const {
    f.write_str("None()");
  }
};

template <class T>
class Option {
  static constexpr auto kTvCopyable = trait::tv_copy_<T>;
  static constexpr auto kTvDroppable = trait::tv_drop_<T>;

  u8 _tag;
  union {
    T _1;
  };

 public:
  constexpr Option() noexcept : _tag{0} {}
  constexpr Option(T val) noexcept : _tag{1}, _1{mem::move(val)} {}

  ~Option() noexcept requires(kTvDroppable) = default;
  ~Option() noexcept {
    switch (_tag) {
      case 0:  break;
      case 1:  mem::drop(_1); break;
      default: break;
    }
  }

  Option(const Option& other) noexcept requires(kTvCopyable) = default;
  Option(Option&& other) noexcept : _tag{0} {
    switch (other._tag) {
      case 0:  break;
      case 1:  ptr::write(&_1, mem::move(other._1)), _tag = 1; break;
      default: break;
    }
  }

  Option(const Option& other) noexcept : _tag{0} {
    switch (other._tag) {
      case 0:  break;
      case 1:  ptr::write(&_1, other._1), _tag = 1; break;
      default: break;
    }
  }

  Option& operator=(const Option& other) noexcept requires(kTvCopyable) = default;
  Option& operator=(Option&& other) noexcept {
    if (this == &other) return *this;
    switch (_tag) {
      case 0:  break;
      case 1:  mem::drop(_1), _tag = 0; break;
      default: break;
    }
    switch (other._tag) {
      case 0:  break;
      case 1:  ptr::write(&_1, mem::move(other._1)), _tag = 1; break;
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

 public:
  auto expect(this auto self, const auto& msg) -> T {
    sfc::expect(self._tag == 1, "Option::expect: {}", msg);
    return mem::move(self._1);
  }

  auto unwrap(this auto self) -> T {
    sfc::expect(self._tag == 1, "Option::unwrap: not Some()");
    return mem::move(self._1);
  }

  auto unwrap_or(this auto self, T default_val) -> T {
    if (self._tag == 1) return mem::move(self._1);
    return mem::move(default_val);
  }

  template <class U>
  auto operator&(this auto self, Option<U> optb) -> Option<U> {
    if (self._tag == 1) return mem::move(optb);
    return {};
  }

  auto operator|(this auto self, Option<T> optb) -> Option<T> {
    if (self._tag == 1) return mem::move(self);
    return mem::move(optb);
  }

  template <class F>
  auto and_then(this auto self, F&& op) -> ops::invoke_t<F(T)> {
    if (self._tag == 1) return op(mem::move(self._1));
    return {};
  }

  auto or_else(this auto self, auto&& f) -> Option<T> {
    if (self._tag == 1) return mem::move(self);
    return f();
  }

  template <class F>
  auto map(this auto self, F&& f) -> Option<ops::invoke_t<F(T)>> {
    if (self._tag == 1) return f(mem::move(self._1));
    return {};
  }

  template <class U>
  auto map_or(this auto self, U default_val, auto&& f) -> U {
    if (self._tag == 1) return f(mem::move(self._1));
    return mem::move(default_val);
  }

  // to result
  template <class E>
  auto ok_or(this auto self, E err) -> result::Result<T, E> {
    if (self._tag == 1) return mem::move(self._1);
    return mem::move(err);
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

template <null_ T>
class Option<T> {
  T _1;

 public:
  constexpr Option() noexcept : _1{} {}
  constexpr Option(T val) noexcept : _1{mem::move(val)} {}

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

 public:
  auto expect(this auto self, const auto& msg) -> T {
    sfc::expect(self._1 != nullptr, "Option::expect: {}", msg);
    return mem::move(self._1);
  }

  auto unwrap(this auto self) -> T {
    sfc::expect(self._1 != nullptr, "Option::unwrap: None()");
    return mem::move(self._1);
  }

  auto unwrap_or(this auto self, T default_val) -> T {
    return self._1 != nullptr ? mem::move(self._1) : mem::move(default_val);
  }

  template <class U>
  auto operator&(this auto self, Option<U> optb) -> Option<U> {
    if (self._1 != nullptr) return mem::move(optb);
    return {};
  }

  auto operator|(this auto self, Option<T> optb) -> Option<T> {
    if (self._1 != nullptr) return mem::move(self);
    return mem::move(optb);
  }

  template <class F>
  auto and_then(this auto self, F&& op) -> ops::invoke_t<F(T)> {
    if (self._1 != nullptr) return op(mem::move(self._1));
    return {};
  }

  auto or_else(this auto self, auto&& f) -> Option<T> {
    if (self._1 != nullptr) return mem::move(self);
    return f();
  }

  template <class F>
  auto map(this auto self, F&& f) -> Option<ops::invoke_t<F(T)>> {
    if (self._1 != nullptr) return {f(mem::move(self._1))};
    return {};
  }

  template <class U>
  auto map_or(this auto self, U default_val, auto&& f) -> U {
    if (self._1 != nullptr) return f(mem::move(self._1));
    return mem::move(default_val);
  }

 public:
  // to result
  template <class E>
  auto ok_or(this auto self, E err) noexcept -> result::Result<T, E> {
    if (self._1 != nullptr) return result::Result<T, E>{mem::move(self._1)};
    return result::Result<T, E>{mem::move(err)};
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

 public:
  auto unwrap_or(this auto self, T& default_val) -> T& {
    return self._1 ? *self._1 : default_val;
  }

  auto unwrap_or_else(this auto self, auto&& f) -> T& {
    if (self._1) return *self._1;
    return f();
  }

  auto expect(this auto self, const auto& msg) -> T& {
    sfc::expect(self._1 != nullptr, "Option::expect: {}", msg);
    return *self._1;
  }

  auto to_owned(this auto self) -> Option<T> {
    if (!self._1) return {};
    return {*(self._1)};
  }

  template <class U>
  auto operator&(this auto self, Option<U> optb) -> Option<U> {
    if (self._1) return mem::move(optb);
    return {};
  }

  auto operator|(this auto self, Option<T> optb) -> Option<T> {
    if (self._1) return mem::move(self);
    return mem::move(optb);
  }

  template <class F>
  auto and_then(this auto self, F&& op) -> ops::invoke_t<F(T&)> {
    if (self._1) return op(*self._1);
    return {};
  }

  auto or_else(this auto self, auto&& f) -> Option<T&> {
    if (self._1) return mem::move(self);
    return f();
  }

  template <class F>
  auto map(this auto self, F&& f) -> Option<ops::invoke_t<F(T&)>> {
    if (self._1) return {f(*self._1)};
    return {};
  }

  template <class U>
  auto map_or(this auto self, U default_val, auto&& f) -> U {
    if (self._1) return f(*self._1);
    return mem::move(default_val);
  }

  // to result
  template <class E>
  auto ok_or(this auto self, E err) -> result::Result<T&, E> {
    if (self._1) return {*self._1};
    return {mem::move(err)};
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
Option(T) -> Option<T>;

Option(const char*) -> Option<str::Str>;

template <class T>
auto operator==(const Option<T>& a, Option<void>) -> bool {
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
using option::Option;
}  // namespace sfc
