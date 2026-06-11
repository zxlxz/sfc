#pragma once

#include "sfc/core/ops.h"
#include "sfc/core/ptr.h"
#include "sfc/core/test.h"

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

 public:
  template <class U>
  auto operator&([[maybe_unused]] Option<U> optb) const -> Option<U> {
    return {};
  }

  template <class U>
  auto operator|(Option<U> optb) const -> Option<U> {
    return mem::move(optb);
  }

  auto and_then([[maybe_unused]] auto&& f) const -> Option<void> {
    return {};
  }

  auto map([[maybe_unused]] auto&& f) const -> Option<void> {
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
    sfc::assert_fmt(_tag == 1, fmt::Args{"Option::operator->: not Some()"});
    return &_1;
  }

  auto operator->() -> T* {
    sfc::assert_fmt(_tag == 1, fmt::Args{"Option::operator->: not Some()"});
    return &_1;
  }

  auto operator*() const -> const T& {
    sfc::assert_fmt(_tag == 1, fmt::Args{"Option::operator*: not Some()"});
    return _1;
  }

  auto operator*() -> T& {
    sfc::assert_fmt(_tag == 1, fmt::Args{"Option::operator*: not Some()"});
    return _1;
  }

 public:
  auto expect(const auto& msg) && -> T {
    sfc::assert_fmt(_tag == 1, fmt::Args{"Option::expect: {}", msg});
    return mem::move(_1);
  }

  auto unwrap() && -> T {
    sfc::assert_fmt(_tag == 1, fmt::Args{"Option::unwrap: not Some()"});
    return mem::move(_1);
  }

  auto unwrap_or(T default_val) && -> T {
    if (_tag == 1) return mem::move(_1);
    return mem::move(default_val);
  }

  template <class U>
  auto operator&(Option<U> optb) && -> Option<U> {
    if (_tag == 1) return mem::move(optb);
    return {};
  }

  auto operator|(Option<T> optb) && -> Option<T> {
    if (_tag == 1) return mem::move(*this);
    return mem::move(optb);
  }

  template <class F, class OptionU = FnOut<F, T>>
  auto and_then(F&& op) && -> OptionU {
    if (_tag == 1) return op(mem::move(_1));
    return {};
  }

  auto or_else(auto&& f) && -> Option<T> {
    if (_tag == 1) return mem::move(*this);
    return f();
  }

  template <class F, class U = FnOut<F, T>>
  auto map(F&& f) && -> Option<U> {
    if (_tag == 1) return f(mem::move(_1));
    return {};
  }

  template <class U>
  auto map_or(U default_val, auto&& f) && -> U {
    if (_tag == 1) return f(mem::move(_1));
    return mem::move(default_val);
  }

 public:
  // to result
  template <class E>
  auto ok_or(E err) && -> result::Result<T, E> {
    if (_tag == 1) return mem::move(_1);
    return mem::move(err);
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    if (_tag == 0) {
      f.write_str("None()");
    } else {
      f.write_fmt(fmt::Args{"Some({})", _1});
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
    sfc::assert_fmt(_1 != nullptr, fmt::Args{"Option::operator->: None()"});
    return &_1;
  }

  auto operator->() -> T* {
    sfc::assert_fmt(_1 != nullptr, fmt::Args{"Option::operator->: None()"});
    return &_1;
  }

  auto operator*() const -> const T& {
    sfc::assert_fmt(_1 != nullptr, fmt::Args{"Option::operator*: None()"});
    return _1;
  }

  auto operator*() -> T& {
    sfc::assert_fmt(_1 != nullptr, fmt::Args{"Option::operator*: None()"});
    return _1;
  }

 public:
  auto expect(const auto& msg) && -> T {
    sfc::assert_fmt(_1 != nullptr, fmt::Args{"Option::expect: {}", msg});
    return mem::move(_1);
  }

  auto unwrap() && -> T {
    sfc::assert_fmt(_1 != nullptr, fmt::Args{"Option::unwrap: None()"});
    return mem::move(_1);
  }

  auto unwrap_or(T default_val) && -> T {
    return _1 != nullptr ? mem::move(_1) : mem::move(default_val);
  }

  template <class U>
  auto operator&(Option<U> optb) && -> Option<U> {
    if (_1 != nullptr) return mem::move(optb);
    return {};
  }

  auto operator|(Option<T> optb) && -> Option<T> {
    if (_1 != nullptr) return mem::move(*this);
    return mem::move(optb);
  }

  template <class F, class OptionU = FnOut<F, T>>
  auto and_then(F&& op) && -> OptionU {
    if (_1 != nullptr) return op(mem::move(_1));
    return {};
  }

  auto or_else(auto&& f) && -> Option<T> {
    if (_1 != nullptr) return mem::move(*this);
    return f();
  }

  template <class F, class U = FnOut<F, T>>
  auto map(F&& f) && -> Option<U> {
    if (_1 != nullptr) return {f(mem::move(_1))};
    return {};
  }

  template <class U>
  auto map_or(U default_val, auto&& f) && -> U {
    if (_1 != nullptr) return f(mem::move(_1));
    return mem::move(default_val);
  }

 public:
  // to result
  template <class E>
  auto ok_or(E err) && -> result::Result<T, E> {
    if (_1 != nullptr) return {mem::move(_1)};
    return {mem::move(err)};
  }

 public:
  // trait: fmt::Display
  void fmt(auto& f) const {
    if (_1 == nullptr) {
      f.write_str("None()");
    } else {
      f.write_fmt(fmt::Args{"Some({})", _1});
    }
  }
};

template <class T>
class Option<T&> {
  T* _1;

 public:
  constexpr Option() noexcept : _1{nullptr} {}
  constexpr Option(T& val) noexcept : _1{&val} {}

  constexpr Option(Option&&) noexcept = default;
  Option& operator=(Option&&) noexcept = default;

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
    sfc::assert_fmt(_1 != nullptr, fmt::Args{"Option::operator->: None()"});
    return _1;
  }

  auto operator->() -> T* {
    sfc::assert_fmt(_1 != nullptr, fmt::Args{"Option::operator->: None()"});
    return _1;
  }

  auto operator*() const -> const T& {
    sfc::assert_fmt(_1 != nullptr, fmt::Args{"Option::deref: nullptr"});
    return *_1;
  }

  auto operator*() -> T& {
    sfc::assert_fmt(_1 != nullptr, fmt::Args{"Option::deref: nullptr"});
    return *_1;
  }

  auto unwrap() const -> T& {
    sfc::assert_fmt(_1 != nullptr, fmt::Args{"Option::unwrap: nullptr"});
    return *_1;
  }

 public:
  auto unwrap_or(T& default_val) && -> T& {
    return _1 ? *_1 : default_val;
  }

  auto unwrap_or_else(auto&& f) && -> T& {
    return _1 ? *_1 : f();
  }

  auto expect(const auto& msg) && -> T& {
    sfc::assert_fmt(_1 != nullptr, fmt::Args{"Option::expect: {}", msg});
    return *_1;
  }

  template <class U>
  auto operator&(Option<U> optb) && -> Option<U> {
    return _1 ? mem::move(optb) : Option<U>{};
  }

  auto operator|(Option<T&> optb) && -> Option<T&> {
    return _1 ? mem::move(*this) : mem::move(optb);
  }

  template <class F, class OptionU = FnOut<F, T&>>
  auto and_then(F&& op) && -> OptionU {
    return _1 ? op(*_1) : OptionU{};
  }

  auto or_else(auto&& f) && -> Option<T&> {
    return _1 ? mem::move(*this) : f();
  }

  template <class F, class U = FnOut<F, T&>>
  auto map(F&& f) && -> Option<U> {
    return _1 ? Option<U>{f(*_1)} : Option<U>{};
  }

  template <class U>
  auto map_or(U default_val, auto&& f) && -> U {
    return _1 ? f(*_1) : mem::move(default_val);
  }

 public:
  auto to_owned() const -> Option<T> {
    return _1 ? Option<T>{*_1} : Option<T>{};
  }

  // to result
  template <class E>
  auto ok_or(E err) && -> result::Result<T&, E> {
    if (_1) return {*_1};
    return {mem::move(err)};
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    if (!_1) {
      f.write_str("None()");
    } else {
      f.write_fmt(fmt::Args{"Some({})", *_1});
    }
  }
};

template <class T>
class Option<const T&> {
  const T* _1;

 public:
  constexpr Option() noexcept : _1{nullptr} {}
  constexpr Option(const T& val) noexcept : _1{&val} {}

  Option(const Option&) noexcept = default;
  Option& operator=(const Option&) noexcept = default;

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
    sfc::assert_fmt(_1 != nullptr, fmt::Args{"Option::operator->: None()"});
    return _1;
  }

  auto operator*() const -> const T& {
    sfc::assert_fmt(_1 != nullptr, fmt::Args{"Option::deref: nullptr"});
    return *_1;
  }

  auto unwrap() const -> const T& {
    sfc::assert_fmt(_1 != nullptr, fmt::Args{"Option::unwrap: nullptr"});
    return *_1;
  }

 public:
  auto unwrap_or(const T& default_val) const -> const T& {
    return _1 ? *_1 : default_val;
  }

  auto unwrap_or_else(auto&& f) const -> const T& {
    return _1 ? *_1 : f();
  }

  auto expect(const auto& msg) const -> const T& {
    sfc::assert_fmt(_1 != nullptr, "Option::expect: {}", msg);
    return *_1;
  }

  template <class U>
  auto operator&(Option<U> optb) const -> Option<U> {
    return _1 ? mem::move(optb) : Option<U>{};
  }

  auto operator|(Option<const T&> optb) const -> Option<const T&> {
    return _1 ? *this : mem::move(optb);
  }

  template <class F, class OptionU = ops::FnOut<F, T>>
  auto and_then(F&& op) const -> OptionU {
    return _1 ? op(*_1) : OptionU{};
  }

  auto or_else(auto&& f) const -> Option<const T&> {
    return _1 ? mem::move(*this) : f();
  }

  template <class F, class U = FnOut<F, T>>
  auto map(F&& f) const -> Option<U> {
    return _1 ? f(*_1) : Option<U>{};
  }

  template <class U>
  auto map_or(U default_val, auto&& f) const -> U {
    if (_1) return f(*_1);
    return mem::move(default_val);
  }

 public:
  auto to_owned() const -> Option<T> {
    return _1 ? Option<T>{*_1} : Option<T>{};
  }

  // to result
  template <class E>
  auto ok_or(E err) const -> result::Result<const T&, E> {
    if (_1) return {*_1};
    return {mem::move(err)};
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    if (!_1) {
      f.write_str("None()");
    } else {
      f.write_fmt(fmt::Args{"Some({})", *_1});
    }
  }
};

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
