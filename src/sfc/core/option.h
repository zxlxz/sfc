#pragma once

#include "sfc/core/ptr.h"
#include "sfc/core/mem.h"
#include "sfc/core/ops.h"
#include "sfc/core/fmts.h"
#include "sfc/core/test.h"

namespace sfc::option {

struct None {
  constexpr auto is_none() const noexcept -> bool {
    return true;
  }

  constexpr auto operator==(const auto& other) const noexcept -> bool {
    return other.is_none();
  }

  void fmt(auto& f) const {
    f.write_str("None()");
  }
};

template <class T>
class OptionStorage {
 protected:
  bool _tag;
  union {
    T _val;
  };

 public:
  OptionStorage() noexcept : _tag{false} {}
  OptionStorage(None) noexcept : _tag{false} {}
  OptionStorage(T val) noexcept : _tag{true}, _val{mem::move(val)} {}

  ~OptionStorage() {
    _tag ? mem::drop(_val) : void();
  }

  OptionStorage(OptionStorage&& other) noexcept : _tag{other._tag} {
    _tag ? ptr::write(&_val, mem::move(other._val)) : void();
  }

  OptionStorage& operator=(OptionStorage&& other) noexcept {
    if (this != &other) {
      if (_tag == other._tag) {
        _tag ? mem::swap(_val, other._val) : void();
      } else {
        _tag ? mem::drop(_val) : void();
        _tag = other._tag;
        _tag ? ptr::write(&_val, mem::move(other._val)) : void();
      }
    }
    return *this;
  }

 public:
  auto is_none() const noexcept -> bool {
    return !_tag;
  }

  auto is_some() const noexcept -> bool {
    return _tag;
  }
};

template <trait::tv_copy_ T>
class OptionStorage<T> {
 protected:
  bool _tag;
  union {
    T _val;
  };

 public:
  OptionStorage() noexcept : _tag{false} {}
  OptionStorage(None) noexcept : _tag{false} {}
  OptionStorage(T val) noexcept : _tag{true}, _val{mem::move(val)} {}

 public:
  auto is_none() const noexcept -> bool {
    return !_tag;
  }

  auto is_some() const noexcept -> bool {
    return _tag;
  }
};

template <class T>
class OptionStorage<T&> {
 protected:
  T* _ptr;

 public:
  constexpr OptionStorage() noexcept : _ptr{nullptr} {}
  constexpr OptionStorage(None) noexcept : _ptr{nullptr} {}
  constexpr OptionStorage(T& val) noexcept : _ptr{&val} {}
  constexpr OptionStorage(T* ptr) noexcept : _ptr{ptr} {}

 public:
  auto is_none() const noexcept -> bool {
    return _ptr == nullptr;
  }

  auto is_some() const noexcept -> bool {
    return _ptr != nullptr;
  }
};

template <class T>
class Option : OptionStorage<T> {
  using Inn = OptionStorage<T>;
  using Inn::_val;

 public:
  using Inn::Inn;
  using Inn::is_none;
  using Inn::is_some;

  explicit operator bool() const noexcept {
    return this->is_some();
  }

  auto operator->() const -> const T* {
    sfc::assert_(this->is_some(), "Option::operator->: deref None()");
    return &_val;
  }

  auto operator->() -> T* {
    sfc::assert_(this->is_some(), "Option::operator->: deref None()");
    return &_val;
  }

  auto operator*() const -> const T& {
    sfc::assert_(this->is_some(), "Option::operator*: deref None()");
    return _val;
  }

  auto operator*() -> T& {
    sfc::assert_(this->is_some(), "Option::operator*: deref None()");
    return _val;
  }

  auto unwrap_unchecked() noexcept -> T {
    return mem::move(_val);
  }

  auto unwrap_err_unchecked() const -> None {
    return {};
  }

 public:
  auto expect(const fmt::Fmts& fmts, const auto&... args) && -> T {
    sfc::assert_(this->is_some(), fmts, args...);
    return mem::move(_val);
  }

  auto unwrap() && -> T {
    sfc::assert_(this->is_some(), "Option::unwrap: not Some()");
    return mem::move(_val);
  }

  auto unwrap_or(T default_val) && -> T {
    if (this->is_some()) return mem::move(_val);
    return mem::move(default_val);
  }

  auto unwrap_or_else(auto&& f) && -> T {
    if (this->is_some()) return mem::move(_val);
    return f();
  }

  template <class U>
  auto operator&(Option<U> optb) -> Option<U> {
    if (this->is_some()) return mem::move(optb);
    return {};
  }

  auto operator|(Option<T> optb) && -> Option<T> {
    if (this->is_some()) return mem::move(*this);
    return mem::move(optb);
  }

  template <class F, class OptionU = ops::FnOut<F, T>>
  auto and_then(F&& op) && -> OptionU {
    if (this->is_some()) return op(mem::move(_val));
    return {};
  }

  auto or_else(auto&& f) && -> Option<T> {
    if (this->is_some()) return mem::move(*this);
    return f();
  }

  template <class F, class U = ops::FnOut<F, T>>
  auto map(F&& f) && -> Option<U> {
    if (this->is_some()) return f(mem::move(_val));
    return {};
  }

  template <class U>
  auto map_or(U default_val, auto&& f) && -> U {
    if (this->is_some()) return f(mem::move(_val));
    return mem::move(default_val);
  }

 public:
  template <class U>
  auto operator==(const Option<U>& other) const -> bool {
    if (this->is_none()) return other.is_none();
    if (other.is_none()) return false;
    return _val == *other;
  }

  // to result
  template <class E>
  auto ok_or(E err) -> result::Result<T, E> {
    if (this->is_some()) return mem::move(_val);
    return mem::move(err);
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    if (this->is_none()) {
      f.write_str("None()");
    } else {
      f.write_fmt("Some({})", _val);
    }
  }
};

template <class T>
class Option<T&> : OptionStorage<T&> {
  using Inn = OptionStorage<T&>;
  using Inn::_ptr;

 public:
  using Inn::Inn;
  using Inn::is_none;
  using Inn::is_some;

  explicit operator bool() const noexcept {
    return _ptr != nullptr;
  }

  auto operator->() const -> const T* {
    return _ptr;
  }

  auto operator->() -> T* {
    return _ptr;
  }

  auto operator*() const -> const T& {
    return *_ptr;
  }

  auto operator*() -> T& {
    return *_ptr;
  }

  auto unwrap_unchecked() noexcept -> T& {
    return *_ptr;
  }

  auto unwrap_err_unchecked() const -> None {
    return {};
  }

 public:
  auto expect(const auto& msg) -> T& {
    sfc::assert_(this->is_some(), "Option::expect: {}", msg);
    return *_ptr;
  }

  auto unwrap() -> T& {
    sfc::assert_(this->is_some(), "Option::unwrap: None()");
    return *_ptr;
  }

  auto unwrap_or(T& default_val) -> T& {
    if (this->is_some()) return *_ptr;
    return default_val;
  }

  auto unwrap_or_else(auto&& f) -> T& {
    if (this->is_some()) return *_ptr;
    return f();
  }

  template <class U>
  auto operator&(Option<U> optb) -> Option<U> {
    if (this->is_some()) return mem::move(optb);
    return {};
  }

  auto operator|(Option<T&> optb) && -> Option<T&> {
    if (this->is_some()) return *this;
    return mem::move(optb);
  }

  template <class F, class OptionU = ops::FnOut<F, T&>>
  auto and_then(F&& op) -> OptionU {
    if (this->is_some()) return op(*_ptr);
    return OptionU{};
  }

  auto or_else(auto&& f) -> Option<T&> {
    if (this->is_some()) return *this;
    return f();
  }

  template <class F, class U = ops::FnOut<F, T&>>
  auto map(F&& f) -> Option<U> {
    if (this->is_some()) return Option<U>{f(*_ptr)};
    return Option<U>{};
  }

  template <class U>
  auto map_or(U default_val, auto&& f) -> U {
    if (this->is_some()) return f(*_ptr);
    return mem::move(default_val);
  }

 public:
  template <class U>
  auto operator==(const Option<U>& other) const -> bool {
    if (this->is_none()) return other.is_none();
    if (other.is_none()) return false;
    return *_ptr == *other;
  }

  auto to_owned() const -> Option<T> {
    if (this->is_none()) return {};
    return T{*_ptr};
  }

  template <class E>
  auto ok_or(E err) -> result::Result<T&, E> {
    if (this->is_some()) return {*_ptr};
    return mem::move(err);
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    if (!this->is_some()) {
      f.write_str("None()");
    } else {
      f.write_fmt("Some({})", *_ptr);
    }
  }
};

template <class T>
class Option<Option<T>>;  // disable

template <class T>
Option(T) -> Option<T>;

template <class T>
Option(T*) -> Option<T&>;

Option(const char*) -> Option<str::Str>;

}  // namespace sfc::option

namespace sfc {
using option::Option;
using option::None;
}  // namespace sfc
