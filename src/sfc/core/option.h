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
  Option(None) noexcept : _tag{false} {}
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
  explicit operator bool() const noexcept {
    return _tag;
  }

  auto is_none() const noexcept -> bool {
    return !_tag;
  }

  auto is_some() const noexcept -> bool {
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

  auto unwrap_unchecked() noexcept -> T {
    return mem::move(_1);
  }

  auto unwrap_err_unchecked() const -> None {
    return {};
  }

 public:
  auto expect(const auto& msg) -> T {
    sfc::assert_(this->is_some(), "Option::expect: {}", msg);
    return mem::move(_1);
  }

  auto unwrap() -> T {
    sfc::assert_(this->is_some(), "Option::unwrap: not Some()");
    return mem::move(_1);
  }

  auto unwrap_or(T default_val) -> T {
    if (this->is_some()) return mem::move(_1);
    return mem::move(default_val);
  }

  auto unwrap_or_else(auto&& f) -> T {
    if (this->is_some()) return mem::move(_1);
    return f();
  }

  template <class U>
  auto operator&(Option<U> optb) -> Option<U> {
    if (this->is_some()) return mem::move(optb);
    return {};
  }

  auto operator|(Option<T> optb) -> Option<T> {
    if (this->is_some()) return mem::move(*this);
    return mem::move(optb);
  }

  template <class F, class OptionU = ops::FnOut<F, T>>
  auto and_then(F&& op) -> OptionU {
    if (this->is_some()) return op(mem::move(_1));
    return {};
  }

  auto or_else(auto&& f) -> Option<T> {
    if (this->is_some()) return mem::move(*this);
    return f();
  }

  template <class F, class U = ops::FnOut<F, T>>
  auto map(F&& f) -> Option<U> {
    if (this->is_some()) return f(mem::move(_1));
    return {};
  }

  template <class U>
  auto map_or(U default_val, auto&& f) -> U {
    if (this->is_some()) return f(mem::move(_1));
    return mem::move(default_val);
  }

 public:
  template <class U>
  auto operator==(const Option<U>& other) const -> bool {
    if (this->is_none()) return other.is_none();
    if (other.is_none()) return false;
    return _1 == *other;
  }

  // to result
  template <class E>
  auto ok_or(E err) -> result::Result<T, E> {
    if (this->is_some()) return mem::move(_1);
    return mem::move(err);
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    if (this->is_none()) {
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

 public:
  explicit operator bool() const noexcept {
    return _1 != nullptr;
  }

  auto is_none() const noexcept -> bool {
    return _1 == nullptr;
  }

  auto is_some() const noexcept -> bool {
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

  auto unwrap_unchecked() noexcept -> T& {
    return *_1;
  }

  auto unwrap_err_unchecked() const -> None {
    return {};
  }

 public:
  auto expect(const auto& msg) -> T& {
    sfc::assert_(this->is_some(), "Option::expect: {}", msg);
    return *_1;
  }

  auto unwrap() -> T& {
    sfc::assert_(this->is_some(), "Option::unwrap: None()");
    return *_1;
  }

  auto unwrap_or(T& default_val) -> T& {
    return this->is_some() ? *_1 : default_val;
  }

  auto unwrap_or_else(auto&& f) -> T& {
    return this->is_some() ? *_1 : f();
  }

  template <class U>
  auto operator&(Option<U> optb) -> Option<U> {
    return this->is_some() ? mem::move(optb) : Option<U>{};
  }

  auto operator|(Option<T&> optb) && -> Option<T&> {
    return this->is_some() ? *this : mem::move(optb);
  }

  template <class F, class OptionU = ops::FnOut<F, T&>>
  auto and_then(F&& op) -> OptionU {
    return this->is_some() ? op(*_1) : OptionU{};
  }

  auto or_else(auto&& f) -> Option<T&> {
    return this->is_some() ? *this : f();
  }

  template <class F, class U = ops::FnOut<F, T&>>
  auto map(F&& f) -> Option<U> {
    return this->is_some() ? Option<U>{f(*_1)} : Option<U>{};
  }

  template <class U>
  auto map_or(U default_val, auto&& f) -> U {
    return this->is_some() ? f(*_1) : mem::move(default_val);
  }

 public:
  template <class U>
  auto operator==(const Option<U>& other) const -> bool {
    if (this->is_none()) return other.is_none();
    if (other.is_none()) return false;
    return *_1 == *other;
  }

  auto to_owned() -> Option<T> {
    return this->is_some() ? Option<T>{*_1} : Option<T>{};
  }

  template <class E>
  auto ok_or(E err) -> result::Result<T&, E> {
    if (this->is_some()) return {*_1};
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
class Option<Option<T>>;

template <class T>
Option(T) -> Option<T>;

template <usize N>
Option(const char (&)[N]) -> Option<str::Str>;

}  // namespace sfc::option

namespace sfc {
using option::None;
using option::Option;
}  // namespace sfc
