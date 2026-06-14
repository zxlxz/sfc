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

  constexpr Inner(Inner&& other) noexcept = default;
  constexpr Inner& operator=(Inner&& other) noexcept = default;

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

template <>
class Inner<void> {
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
};

template <class T>
class Option;

template <class T>
class Option : Inner<T> {
  using Inn = Inner<T>;

 public:
  using Inn::Inn;
  using Inn::is_some;
  using Inn::is_none;
  using Inn::operator=;
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
  auto expect(this auto self, const auto& msg) -> T {
    sfc::assert_fmt(self.is_some(), fmt::Args{"Option::expect: {}", msg});
    return mem::move(self._1);
  }

  auto unwrap(this auto self) -> T {
    sfc::assert_fmt(self.is_some(), fmt::Args{"Option::unwrap: not Some()"});
    return mem::move(self._1);
  }

  auto unwrap_or(this auto self, T default_val) -> T {
    if (self.is_some()) return mem::move(self._1);
    return mem::move(default_val);
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

  template <class F, class OptionU = FnOut<F, T>>
  auto and_then(this auto self, F&& op) -> OptionU {
    if (self.is_some()) return op(mem::move(self._1));
    return {};
  }

  auto or_else(this auto self, auto&& f) -> Option<T> {
    if (self.is_some()) return mem::move(self);
    return f();
  }

  template <class F, class U = FnOut<F, T>>
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
      f.write_fmt(fmt::Args{"Some({})", self._1});
    }
  }
};

template <class T>
class Option<T&> : Inner<T&> {
  using Inn = Inner<T&>;
  using Inn::_1;

 public:
  using Inn::Inn;
  using Inn::is_some;
  using Inn::is_none;
  using Inn::operator=;
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

 public:
  auto expect(this auto self, const auto& msg) -> T& {
    sfc::assert_fmt(self.is_some(), fmt::Args{"Option::expect: {}", msg});
    return *self._1;
  }

  auto unwrap(this auto self) -> T& {
    sfc::assert_fmt(self.is_some(), fmt::Args{"Option::unwrap: None()"});
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

  template <class F, class OptionU = FnOut<F, T&>>
  auto and_then(F&& op) && -> OptionU {
    return this->is_some() ? op(*_1) : OptionU{};
  }

  auto or_else(this auto self, auto&& f) -> Option<T&> {
    return self.is_some() ? mem::move(self) : f();
  }

  template <class F, class U = FnOut<F, T&>>
  auto map(this auto self, F&& f) -> Option<U> {
    return self.is_some() ? Option<U>{f(*self._1)} : Option<U>{};
  }

  template <class U>
  auto map_or(this auto self, U default_val, auto&& f) -> U {
    return self.is_some() ? f(*self._1) : mem::move(default_val);
  }

 public:
  auto to_owned(this auto self) -> Option<T> {
    return self.is_some() ? Option<T>{*self._1} : Option<T>{};
  }

  // to result
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
  using Inn::is_some;
  using Inn::is_none;
  using Inn::operator=;
  using Inn::operator bool;

  auto operator->() const -> const T* {
    return _1;
  }

  auto operator*() const -> const T& {
    return *_1;
  }

 public:
  auto expect(this auto self, const auto& msg) -> const T& {
    sfc::assert_fmt(self.is_some(), fmt::Args{"Option::expect: {}", msg});
    return *self._1;
  }

  auto unwrap(this auto self) -> const T& {
    sfc::assert_fmt(self.is_some(), fmt::Args{"Option::unwrap: None()"});
    return *self._1;
  }

  auto unwrap_or(this auto self, const T& default_val) -> const T& {
    return self.is_some() ? *self._1 : default_val;
  }

  auto unwrap_or_else(this auto self, auto&& f) -> const T& {
    return self.is_some() ? *self._1 : f();
  }

  template <class U>
  auto operator&(this auto self, Option<U> optb) -> Option<U> {
    return self.is_some() ? mem::move(optb) : Option<U>{};
  }

  auto operator|(this auto self, Option optb) -> Option {
    return self.is_some() ? self : optb;
  }

  template <class F, class OptionU = FnOut<F, const T&>>
  auto and_then(this auto self, F&& op) -> OptionU {
    return self.is_some() ? op(*self._1) : OptionU{};
  }

  auto or_else(this auto self, auto&& f) -> Option {
    return self.is_some() ? self : f();
  }

  template <class F, class U = FnOut<F, const T&>>
  auto map(this auto self, F&& f) -> Option<U> {
    return self.is_some() ? Option<U>{f(*self._1)} : Option<U>{};
  }

  template <class U>
  auto map_or(this auto self, U default_val, auto&& f) -> U {
    return self.is_some() ? f(*self._1) : mem::move(default_val);
  }

 public:
  auto to_owned(this auto self) -> Option<T> {
    return self.is_some() ? Option<T>{*self._1} : Option<T>{};
  }

  // to result
  template <class E>
  auto ok_or(this auto self, E err) -> result::Result<const T&, E> {
    if (self.is_some()) return result::Result<const T&, E>{*self._1};
    return result::Result<const T&, E>{mem::move(err)};
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    if (!this->is_some()) {
      f.write_str("None()");
    } else {
      f.write_fmt(fmt::Args{"Some({})", *this->_1});
    }
  }
};

template <>
class Option<void> : Inner<void> {
  using Inn = Inner<void>;

 public:
  using Inn::Inn;
  using Inn::is_some;
  using Inn::is_none;
  using Inn::operator=;
  using Inn::operator bool;

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
  if constexpr (!trait::same_<A, void> && !trait::same_<B, void>) {
    if (a.is_some() && b.is_some()) {
      return *a == *b;
    }
  }
  return a.is_none() && b.is_none();
}

}  // namespace sfc::option

namespace sfc {
using option::Option;
}  // namespace sfc
