#pragma once

#include "sfc/core/expect.h"
#include "sfc/core/ops.h"
#include "sfc/core/ptr.h"

namespace sfc::option {

struct None {};
struct Some {};

template <class T>
concept option_ = requires(const T& x) { x.ptr(); };

template <class T>
class Option {
  bool _tag;
  union {
    T _val;
  };

 public:
  constexpr Option() noexcept : _tag{false} {}
  constexpr Option(T val) noexcept : _tag{true}, _val{static_cast<T&&>(val)} {}

  constexpr Option(None) noexcept : _tag{false} {}
  constexpr Option(Some, auto&&... args) noexcept : _tag{true}, _val{static_cast<decltype(args)&&>(args)...} {}

  constexpr Option(const Option& other) noexcept : _tag{other._tag} {
    if constexpr (trait::tv_copy_<T>) {
      _val = other._val;
    } else {
      if (_tag) ptr::write(&_val, other._val);
    }
  }

  constexpr Option(Option&& other) noexcept : _tag{other._tag} {
    if (_tag) ptr::write(&_val, mem::move(other._val));
  }

  ~Option() noexcept {
    if constexpr (!trait::tv_copy_<T>) {
      if (_tag) _val.~T();
    }
  }

  Option& operator=(const Option& other) noexcept {
    if (this != &other) {
      if (_tag) _val.~T();
      if ((_tag = other._tag)) ptr::write(&_val, other._val);
    }
    return *this;
  }

  Option& operator=(Option&& other) noexcept {
    if (this != &other) {
      if (_tag) _val.~T();
      if ((_tag = other._tag)) ptr::write(&_val, static_cast<T&&>(other._val));
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

  auto operator->() const noexcept -> const T* {
    sfc::expect(_tag, "Option::operator->: not Some()");
    return &_val;
  }

  auto operator->() noexcept -> T* {
    sfc::expect(_tag, "Option::operator->: not Some()");
    return &_val;
  }

  auto operator*() const noexcept -> const T& {
    sfc::expect(_tag, "Option::operator*: not Some()");
    return _val;
  }

  auto operator*() noexcept -> T& {
    sfc::expect(_tag, "Option::operator*: not Some()");
    return _val;
  }

  auto unwrap() && noexcept -> T {
    sfc::expect(_tag, "Option::unwrap: not Some()");
    return static_cast<T&&>(_val);
  }

  auto unwrap_or(this auto self, T default_val) noexcept -> T {
    return self._tag ? static_cast<T&&>(self._val) : static_cast<T&&>(default_val);
  }

  auto unwrap_unchecked() noexcept -> T {
    return static_cast<T&&>(_val);
  }

  auto expect(this auto&& self, const auto& msg) noexcept -> T {
    sfc::expect(bool(self._tag), "Option::expect: {}", msg);
    return static_cast<T&&>(self._val);
  }

  template <class U>
  auto operator&(Option<U> optb) && noexcept -> Option<U> {
    return _tag ? static_cast<Option<U>&&>(optb) : Option<U>{};
  }

  auto operator|(Option<T> optb) && noexcept -> Option<T> {
    return _tag ? static_cast<Option<T>&&>(*this) : static_cast<Option<T>&&>(optb);
  }

  template <class F>
  auto and_then(F&& op) && noexcept -> ops::invoke_t<F(T)> {
    using OptionU = ops::invoke_t<F(T)>;
    return _tag ? op(static_cast<T&&>(_val)) : OptionU{};
  }

  auto or_else(auto&& f) && noexcept -> Option<T> {
    return _tag ? static_cast<Option<T>&&>(*this) : f();
  }

  template <class F>
  auto map(this auto self, F&& f) -> Option<ops::invoke_t<F(T)>> {
    using U = ops::invoke_t<F(T)>;
    return self._tag ? Option<U>{f(static_cast<T&&>(self._val))} : Option<U>{};
  }

  template <class U>
  auto map_or(this auto self, U default_val, auto&& f) -> U {
    return self._tag ? f(static_cast<T&&>(self._val)) : static_cast<U&&>(default_val);
  }

 public:
  // to result
  template <class E>
  auto ok_or(this auto self, E err) -> result::Result<T, E> {
    if (self._tag) {
      return {static_cast<T&&>(self._val)};
    }
    return {static_cast<E&&>(err)};
  }

 public:
  // trait: ops::Eq
  template <class U>
  auto operator==(const Option<U>& other) const -> bool {
    if (!_tag) {
      return !bool(other);
    } else {
      return bool(other) && (_val == *other);
    }
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    if (!_tag) {
      f.write_str("None()");
    } else {
      f.write_fmt("Some({})", _val);
    }
  }
};

template <option_ T>
class Option<T> {
  T _inn;

 public:
  constexpr Option() noexcept : _inn{} {}
  constexpr Option(T val) noexcept : _inn{static_cast<T&&>(val)} {}

  constexpr Option(None) noexcept : _inn{} {}
  constexpr Option(Some, auto&&... args) noexcept : _inn{static_cast<decltype(args)&&>(args)...} {}

  constexpr auto is_some() const noexcept -> bool {
    return _inn.ptr() != nullptr;
  }

  constexpr auto is_none() const noexcept -> bool {
    return _inn.ptr() == nullptr;
  }

  constexpr explicit operator bool() const noexcept {
    return _inn.ptr() != nullptr;
  }

  auto operator->() const noexcept -> const T* {
    sfc::expect(bool(_inn.ptr()), "Option::operator->: None()");
    return &_inn;
  }

  auto operator->() noexcept -> T* {
    sfc::expect(bool(_inn.ptr()), "Option::operator->: None()");
    return &_inn;
  }

  auto operator*() const noexcept -> const T& {
    sfc::expect(bool(_inn.ptr()), "Option::operator*: None()");
    return _inn;
  }

  auto operator*() noexcept -> T& {
    sfc::expect(bool(_inn.ptr()), "Option::operator*: None()");
    return _inn;
  }

  auto unwrap(this auto self) noexcept -> T {
    sfc::expect(bool(self._inn.ptr()), "Option::unwrap: None()");
    return static_cast<T&&>(self._inn);
  }

  auto unwrap_or(this auto self, T default_val) noexcept -> T {
    return self._inn.ptr() ? static_cast<T&&>(self._inn) : static_cast<T&&>(default_val);
  }

  auto unwrap_unchecked(this auto self) noexcept -> T {
    return static_cast<T&&>(self._inn);
  }

  auto expect(this auto self, const auto& msg) noexcept -> T {
    sfc::expect(bool(self._inn.ptr()), "Option::expect: {}", msg);
    return static_cast<T&&>(self._inn);
  }

  template <class U>
  auto operator&(this auto self, Option<U> optb) noexcept -> Option<U> {
    return self._inn.ptr() ? static_cast<Option<U>&&>(optb) : Option<U>{};
  }

  auto operator|(this auto self, Option<T> optb) noexcept -> Option<T> {
    return static_cast<Option<T>&&>(self._inn.ptr() ? self : optb);
  }

  template <class F>
  auto and_then(this auto self, F&& op) noexcept -> ops::invoke_t<F(T)> {
    using OptionU = ops::invoke_t<F(T)>;
    return self.ptr() ? op(static_cast<T&&>(self._inn)) : OptionU{};
  }

  auto or_else(this auto self, auto&& f) noexcept -> Option<T> {
    return self._inn.ptr() ? static_cast<Option<T>&&>(self) : f();
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
    if (self._inn.ptr()) {
      return {static_cast<T&&>(self._inn)};
    }
    return {static_cast<E&&>(err)};
  }

 public:
  // trait: ops::Eq
  template <class U>
  auto operator==(const Option<U>& other) const -> bool {
    if (!_inn.ptr()) {
      return !other;
    } else {
      return other && (_inn == *other);
    }
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    if (!_inn.ptr()) {
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

  constexpr Option(None) noexcept : _ptr{nullptr} {}
  constexpr Option(Some, T& val) noexcept : _ptr{&val} {}

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
  auto ok_or(E err) && -> result::Result<T, E> {
    if (_ptr) {
      return {*_ptr};
    }
    return {static_cast<E&&>(err)};
  }

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
