#pragma once

#include "sfc/core/ops.h"
#include "sfc/core/ptr.h"
#include "sfc/core/test.h"

namespace sfc::option {

template <class T>
concept none_ = requires(const T& x) { x.is_none(); };

template <class T>
class Inner {
 protected:
  u8 _tag;
  union {
    T _1;
  };

 public:
  Inner() noexcept : _tag{0} {}
  Inner(T val) noexcept : _tag{1}, _1{mem::move(val)} {}

  ~Inner() noexcept requires(trait::tv_drop_<T>) = default;
  ~Inner() noexcept {
    if (_tag == 1) mem::drop(_1);
  }

  Inner(const Inner& other) requires(trait::tv_copy_<T>) = default;
  Inner(Inner&& other) noexcept : _tag{other._tag} {
    if (_tag == 1) ptr::write(&_1, mem::move(other._1));
  }

  Inner& operator=(const Inner& other) requires(trait::tv_copy_<T>) = default;
  Inner& operator=(Inner&& other) noexcept {
    if (this == &other) return *this;
    if (_tag == 1) mem::drop(_1);
    _tag = other._tag;
    if (_tag == 1) ptr::write(&_1, mem::move(other._1));
    return *this;
  }

  constexpr auto is_some() const noexcept -> bool {
    return _tag == 1;
  }

  constexpr auto is_none() const noexcept -> bool {
    return _tag == 0;
  }

  constexpr operator bool() const noexcept {
    return _tag == 1;
  }
};

template <none_ T>
class Inner<T> {
 protected:
  T _1;

 public:
  Inner() noexcept : _1{} {}
  Inner(T val) noexcept : _1{mem::move(val)} {}

  constexpr auto is_some() const noexcept -> bool {
    return !_1.is_none();
  }

  constexpr auto is_none() const noexcept -> bool {
    return _1.is_none();
  }

  constexpr operator bool() const noexcept {
    return !_1.is_none();
  }
};

template <class T>
class Inner<T&> {
 protected:
  T* _1;

 public:
  constexpr Inner() noexcept : _1{nullptr} {}
  constexpr Inner(T& val) noexcept : _1{&val} {}

  constexpr auto is_some() const noexcept -> bool {
    return _1 != nullptr;
  }

  constexpr auto is_none() const noexcept -> bool {
    return _1 == nullptr;
  }

  constexpr explicit operator bool() const noexcept {
    return _1 != nullptr;
  }
};

template <class T>
class Inner<const T&> {
 protected:
  const T* _1;

 public:
  constexpr Inner() noexcept : _1{nullptr} {}
  constexpr Inner(const T& val) noexcept : _1{&val} {}

  constexpr auto is_some() const noexcept -> bool {
    return _1 != nullptr;
  }

  constexpr auto is_none() const noexcept -> bool {
    return _1 == nullptr;
  }

  constexpr explicit operator bool() const noexcept {
    return _1 != nullptr;
  }
};

template <class T>
class Option;

template <class T>
class Option : Inner<T> {
  using Inn = Inner<T>;

 public:
  using Inn::Inn;
  using Inn::operator=;

  using Inn::is_some;
  using Inn::is_none;
  using Inn::operator bool;

  auto operator->() const -> const T* {
    sfc::assert_fmt(this->is_some(), fmt::Args{"Option::operator->: deref None()"});
    return &this->_1;
  }

  auto operator->() -> T* {
    sfc::assert_fmt(this->is_some(), fmt::Args{"Option::operator->: deref None()"});
    return &this->_1;
  }

  auto operator*() const -> const T& {
    sfc::assert_fmt(this->is_some(), fmt::Args{"Option::operator*: deref None()"});
    return this->_1;
  }

  auto operator*() -> T& {
    sfc::assert_fmt(this->is_some(), fmt::Args{"Option::operator*: deref None()"});
    return this->_1;
  }

 public:
  auto expect(const auto& msg) && -> T {
    sfc::assert_fmt(this->is_some(), fmt::Args{"Option::expect: {}", msg});
    return mem::move(this->_1);
  }

  auto unwrap() && -> T {
    sfc::assert_fmt(this->is_some(), fmt::Args{"Option::unwrap: not Some()"});
    return mem::move(this->_1);
  }

  auto unwrap_or(T default_val) && -> T {
    if (this->is_some()) return mem::move(this->_1);
    return mem::move(default_val);
  }

  template <class U>
  auto operator&(Option<U> optb) && -> Option<U> {
    if (this->is_some()) return mem::move(optb);
    return {};
  }

  auto operator|(Option<T> optb) && -> Option<T> {
    if (this->is_some()) return mem::move(*this);
    return mem::move(optb);
  }

  template <class F, class OptionU = FnOut<F, T>>
  auto and_then(F&& op) && -> OptionU {
    if (this->is_some()) return op(mem::move(this->_1));
    return {};
  }

  auto or_else(auto&& f) && -> Option<T> {
    if (this->is_some()) return mem::move(*this);
    return f();
  }

  template <class F, class U = FnOut<F, T>>
  auto map(F&& f) && -> Option<U> {
    if (this->is_some()) return f(mem::move(this->_1));
    return {};
  }

  template <class U>
  auto map_or(U default_val, auto&& f) && -> U {
    if (this->is_some()) return f(mem::move(this->_1));
    return mem::move(default_val);
  }

 public:
  // to result
  template <class E>
  auto ok_or(E err) && -> result::Result<T, E> {
    if (this->is_some()) return mem::move(this->_1);
    return mem::move(err);
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    if (this->is_none()) {
      f.write_str("None()");
    } else {
      f.write_fmt(fmt::Args{"Some({})", this->_1});
    }
  }
};

template <class T>
class Option<T&> : Inner<T&> {
  using Inn = Inner<T&>;
  using Inn::_1;

 public:
  using Inn::Inn;
  using Inn::operator=;
  using Inn::is_some;
  using Inn::is_none;
  using Inn::operator bool;

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

  auto unwrap() && -> T& {
    sfc::assert_fmt(_1, fmt::Args{"Option::unwrap: None()"});
    return *_1;
  }

 public:
  auto unwrap_or(T& default_val) && -> T& {
    return this->is_some() ? *_1 : default_val;
  }

  auto unwrap_or_else(auto&& f) && -> T& {
    return this->is_some() ? *_1 : f();
  }

  template <class U>
  auto operator&(Option<U> optb) && -> Option<U> {
    return this->is_some() ? mem::move(optb) : Option<U>{};
  }

  auto operator|(Option<T&> optb) && -> Option<T&> {
    return this->is_some() ? mem::move(*this) : mem::move(optb);
  }

  template <class F, class OptionU = FnOut<F, T&>>
  auto and_then(F&& op) && -> OptionU {
    return this->is_some() ? op(*_1) : OptionU{};
  }

  auto or_else(auto&& f) && -> Option<T&> {
    return this->is_some() ? mem::move(*this) : f();
  }

  template <class F, class U = FnOut<F, T&>>
  auto map(F&& f) && -> Option<U> {
    return this->is_some() ? Option<U>{f(*_1)} : Option<U>{};
  }

  template <class U>
  auto map_or(U default_val, auto&& f) && -> U {
    return this->is_some() ? f(*_1) : mem::move(default_val);
  }

 public:
  auto to_owned() && -> Option<T> {
    return this->is_some() ? Option<T>{*_1} : Option<T>{};
  }

  // to result
  template <class E>
  auto ok_or(E err) && -> result::Result<T&, E> {
    if (this->is_some()) return {*_1};
    return {mem::move(err)};
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    if (!this->is_some()) {
      f.write_str("None()");
    } else {
      f.write_fmt(fmt::Args{"Some({})", *_1});
    }
  }
};

template <class T>
class Option<const T&> : Inner<const T&> {
  using Inn = Inner<const T&>;
  using Inn::_1;

 public:
  using Inn::Inn;
  using Inn::operator=;
  using Inn::is_some;
  using Inn::is_none;
  using Inn::operator bool;

  auto operator->() const -> const T* {
    return _1;
  }

  auto operator*() const -> const T& {
    return *_1;
  }

  auto unwrap() const -> const T& {
    sfc::assert_fmt(_1 != nullptr, fmt::Args{"Option::unwrap: None()"});
    return *_1;
  }

 public:
  auto unwrap_or(const T& default_val) const -> const T& {
    return this->is_some() ? *_1 : default_val;
  }

  auto unwrap_or_else(auto&& f) const -> const T& {
    return this->is_some() ? *_1 : f();
  }

  template <class U>
  auto operator&(Option<U> optb) const -> Option<U> {
    return this->is_some() ? mem::move(optb) : Option<U>{};
  }

  auto operator|(Option optb) const -> Option {
    return this->is_some() ? *this : optb;
  }

  template <class F, class OptionU = FnOut<F, const T&>>
  auto and_then(F&& op) const -> OptionU {
    return this->is_some() ? op(*_1) : OptionU{};
  }

  auto or_else(auto&& f) const -> Option {
    return this->is_some() ? *this : f();
  }

  template <class F, class U = FnOut<F, const T&>>
  auto map(F&& f) const -> Option<U> {
    return this->is_some() ? Option<U>{f(*_1)} : Option<U>{};
  }

  template <class U>
  auto map_or(U default_val, auto&& f) const -> U {
    return this->is_some() ? f(*_1) : mem::move(default_val);
  }

 public:
  auto to_owned() const -> Option<T> {
    return this->is_some() ? Option<T>{*_1} : Option<T>{};
  }

  // to result
  template <class E>
  auto ok_or(E err) const -> result::Result<const T&, E> {
    if (this->is_some()) return result::Result<const T&, E>{*_1};
    return result::Result<const T&, E>{mem::move(err)};
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    if (!this->is_some()) {
      f.write_str("None()");
    } else {
      f.write_fmt(fmt::Args{"Some({})", *_1});
    }
  }
};

template <>
class Option<void> {
 public:
  constexpr auto is_some() const noexcept -> bool {
    return false;
  }

  constexpr auto is_none() const noexcept -> bool {
    return true;
  }

  constexpr operator bool() const noexcept {
    return false;
  }

 public:
  template <class U>
  auto operator&([[maybe_unused]] Option<U> optb) const -> Option<U> {
    return {};
  }

  template <class U>
  auto operator|(Option<U> optb) const -> Option<U> {
    return mem::move(optb);
  }

  auto and_then([[maybe_unused]] auto&& f) const -> Option {
    return {};
  }

  auto map([[maybe_unused]] auto&& f) const -> Option {
    return {};
  }

  template <class U>
  auto map_or(U default_val, [[maybe_unused]] auto&& f) const -> U {
    return default_val;
  }

  template <class F, class OptionU = FnOut<F>>
  auto or_else(F&& f) const -> OptionU {
    return f();
  }

 public:
  void fmt(auto& f) const {
    f.write_str("None()");
  }
};

Option() -> Option<void>;

template <class T>
Option(T) -> Option<T>;

template <usize N>
Option(const char (&)[N]) -> Option<str::Str>;

template <class A, class B>
auto operator==(const Option<A>& a, const Option<B>& b) -> bool {
  if constexpr (trait::same_<A, void>) {
    return b.is_none();
  } else if constexpr (trait::same_<B, void>) {
    return a.is_none();
  } else {
    if (a.is_some() && b.is_some()) {
      return *a == *b;
    }
    return a.is_none() && b.is_none();
  }
}

}  // namespace sfc::option

namespace sfc {
using option::Option;
}  // namespace sfc
