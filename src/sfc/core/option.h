#pragma once

#include "sfc/core/ptr.h"
#include "sfc/core/mem.h"
#include "sfc/core/ops.h"
#include "sfc/core/test.h"
#include "sfc/core/tuple.h"

namespace sfc::option {

template <class T>
class Option;

struct None {
  template <class T>
  constexpr auto operator==(const Option<T>& other) const noexcept -> bool {
    return other.is_none();
  }

  void fmt(auto& f) const {
    f.write_str("None()");
  }
};

template <class T>
class Option {
  bool _tag;
  union {
    T _1;
  };

 public:
  Option() noexcept : _tag{false} {}
  Option(T val) noexcept : _tag{true}, _1{mem::move(val)} {}

  ~Option() requires(trait::tv_drop_<T>) = default;
  ~Option() {
    if (_tag) mem::drop(_1);
  }

  Option(const Option& other) requires(trait::tv_copy_<T>) = default;
  Option(Option&& other) noexcept : _tag{other._tag} {
    if (_tag) ptr::write(&_1, mem::move(other._1));
  }

  Option& operator=(const Option& other) requires(trait::tv_copy_<T>) = default;
  Option& operator=(Option&& other) noexcept {
    if (this == &other) return *this;
    if (_tag) mem::drop(_1);
    _tag = other._tag;
    if (_tag) ptr::write(&_1, mem::move(other._1));
    return *this;
  }

 public:
  auto is_none() const noexcept -> bool {
    return !_tag;
  }

  auto is_some() const noexcept -> bool {
    return _tag;
  }

  operator bool() const noexcept {
    return _tag;
  }

  auto operator->() const -> const T* {
    sfc::assert_(this->is_some(), "Option::operator->: deref None()");
    return &_1;
  }

  auto operator->() -> T* {
    sfc::assert_(this->is_some(), "Option::operator->: deref None()");
    return &_1;
  }

  auto operator*() const -> const T& {
    sfc::assert_(this->is_some(), "Option::operator*: deref None()");
    return _1;
  }

  auto operator*() -> T& {
    sfc::assert_(this->is_some(), "Option::operator*: deref None()");
    return _1;
  }

 public:
  auto expect(this auto self, const auto& msg) -> T {
    sfc::assert_(self.is_some(), "Option::expect: {}", msg);
    return mem::move(self._1);
  }

  auto unwrap(this auto self) -> T {
    sfc::assert_(self.is_some(), "Option::unwrap: not Some()");
    return mem::move(self._1);
  }

  auto unwrap_or(this auto self, T default_val) -> T {
    if (self.is_some()) return mem::move(self._1);
    return mem::move(default_val);
  }

  auto unwrap_or_else(this auto self, auto&& f) -> T {
    if (self.is_some()) return mem::move(self._1);
    return f();
  }

  template <class U>
  auto operator&(this auto self, Option<U> optb) -> Option<U> {
    if (self.is_some()) return mem::move(optb);
    return {};
  }

  auto operator|(this auto self, Option<T> optb) -> Option<T> {
    if (self.is_some()) return mem::move(self);
    return mem::move(optb);
  }

  template <class F, class OptionU = ops::FnOut<F, T>>
  auto and_then(this auto self, F&& op) -> OptionU {
    if (self.is_some()) return op(mem::move(self._1));
    return {};
  }

  auto or_else(this auto self, auto&& f) -> Option<T> {
    if (self.is_some()) return mem::move(self);
    return f();
  }

  template <class F, class U = ops::FnOut<F, T>>
  auto map(this auto self, F&& f) -> Option<U> {
    if (self.is_some()) return f(mem::move(self._1));
    return {};
  }

  template <class U>
  auto map_or(this auto self, U default_val, auto&& f) -> U {
    if (self.is_some()) return f(mem::move(self._1));
    return mem::move(default_val);
  }

 public:
  auto operator==(const Option& other) const -> bool {
    if (this->is_none()) return other.is_none();
    if (other.is_none()) return false;
    return _1 == *other;
  }

  // to result
  template <class E>
  auto ok_or(this auto self, E err) -> result::Result<T, E> {
    if (self.is_some()) return mem::move(self._1);
    return mem::move(err);
  }

  // trait: fmt::Display
  void fmt(this const auto& self, auto& f) {
    if (self.is_none()) {
      f.write_str("None()");
    } else {
      f.write_fmt("Some({})", self._1);
    }
  }
};

template <class T>
class Option<T&> {
  T* _1;

 public:
  constexpr Option() noexcept : _1{nullptr} {}
  constexpr Option(T& val) noexcept : _1{&val} {}

  auto is_none() const noexcept -> bool {
    return _1 == nullptr;
  }

  auto is_some() const noexcept -> bool {
    return _1 != nullptr;
  }

  operator bool() const noexcept {
    return _1 != nullptr;
  }

  auto operator->() const -> const T* {
    return _1;
  }

  auto operator->() -> T* {
    return _1;
  }

  auto operator*() const -> const T& {
    return *_1;
  }

  auto operator*() -> T& {
    return *_1;
  }

 public:
  auto expect(this auto self, const auto& msg) -> T& {
    sfc::assert_(self.is_some(), "Option::expect: {}", msg);
    return *self._1;
  }

  auto unwrap(this auto self) -> T& {
    sfc::assert_(self.is_some(), "Option::unwrap: None()");
    return *self._1;
  }

  auto unwrap_or(this auto self, T& default_val) -> T& {
    return self.is_some() ? *self._1 : default_val;
  }

  auto unwrap_or_else(this auto self, auto&& f) -> T& {
    return self.is_some() ? *self._1 : f();
  }

  template <class U>
  auto operator&(this auto self, Option<U> optb) -> Option<U> {
    return self.is_some() ? mem::move(optb) : Option<U>{};
  }

  auto operator|(this auto self, Option<T&> optb) -> Option<T&> {
    return self.is_some() ? mem::move(self) : mem::move(optb);
  }

  template <class F, class OptionU = ops::FnOut<F, T&>>
  auto and_then(F&& op) && -> OptionU {
    return this->is_some() ? op(*_1) : OptionU{};
  }

  auto or_else(this auto self, auto&& f) -> Option<T&> {
    return self.is_some() ? mem::move(self) : f();
  }

  template <class F, class U = ops::FnOut<F, T&>>
  auto map(this auto self, F&& f) -> Option<U> {
    return self.is_some() ? Option<U>{f(*self._1)} : Option<U>{};
  }

  template <class U>
  auto map_or(this auto self, U default_val, auto&& f) -> U {
    return self.is_some() ? f(*self._1) : mem::move(default_val);
  }

 public:
  auto operator==(const Option& other) const -> bool {
    if (this->is_none()) return other.is_none();
    if (other.is_none()) return false;
    return *_1 == *other;
  }

  auto to_owned(this auto self) -> Option<T> {
    return self.is_some() ? Option<T>{*self._1} : Option<T>{};
  }

  template <class E>
  auto ok_or(this auto self, E err) -> result::Result<T&, E> {
    if (self.is_some()) return {*self._1};
    return {mem::move(err)};
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    if (!this->is_some()) {
      f.write_str("None()");
    } else {
      f.write_fmt("Some({})", *_1);
    }
  }
};

template <class T>
Option(T) -> Option<T>;

template <usize N>
Option(const char (&)[N]) -> Option<str::Str>;

}  // namespace sfc::option

namespace sfc {
using option::None;
using option::Option;
}  // namespace sfc
