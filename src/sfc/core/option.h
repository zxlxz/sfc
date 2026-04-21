#pragma once

#include "sfc/core/ops.h"
#include "sfc/core/ptr.h"
#include "sfc/core/panic.h"

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
    if (this != &other) {
      if (_tag) {
        _val.~T();
        _tag = false;
      }
      if (other._tag) {
        ptr::write(&_val, other._val);
        _tag = other._tag;
      }
    }
    return *this;
  }

  Option& operator=(Option&& other) noexcept {
    if (this != &other) {
      if (_tag) {
        _val.~T();
        _tag = false;
      }
      if (other._tag) {
        ptr::write(&_val, static_cast<T&&>(other._val));
        _tag = other._tag;
      }
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

  auto unwrap_or(T default_val) && noexcept -> T {
    return _tag ? static_cast<T&&>(_val) : static_cast<T&&>(default_val);
  }

  auto unwrap_unchecked() noexcept -> T {
    return static_cast<T&&>(_val);
  }

  auto expect(const auto& msg) && noexcept -> T {
    sfc::expect(bool(_tag), "Option::expect: {}", msg);
    return static_cast<T&&>(_val);
  }

  template <class U>
  auto operator&(Option<U> optb) && noexcept -> Option<U> {
    if (_tag) {
      return static_cast<Option<U>&&>(optb);
    }
    return {};
  }

  auto operator|(Option<T> optb) && noexcept -> Option<T> {
    if (_tag) {
      return static_cast<Option<T>&&>(*this);
    }
    return static_cast<Option<T>&&>(optb);
  }

  template <class F>
  auto and_then(F&& op) && noexcept -> ops::invoke_t<F(T)> {
    if (_tag) {
      return op(static_cast<T&&>(_val));
    }
    return {};
  }

  auto or_else(auto&& f) && noexcept -> Option<T> {
    if (_tag) {
      return static_cast<Option<T>&&>(*this);
    }
    return f();
  }

  template <class F>
  auto map(F&& f) && -> Option<ops::invoke_t<F(T)>> {
    if (_tag) {
      return f(static_cast<T&&>(_val));
    }
    return {};
  }

  template <class U>
  auto map_or(U default_val, auto&& f) && -> U {
    if (_tag) {
      return f(static_cast<T&&>(_val));
    }
    return static_cast<U&&>(default_val);
  }

 public:
  // to result
  template <class E>
  auto ok_or(E err) && noexcept -> result::Result<T, E> {
    if (_tag) {
      return static_cast<T&&>(_val);
    }
    return static_cast<E&&>(err);
  }

 public:
  // trait: ops::Eq
  template <class U>
  auto operator==(const Option<U>& other) const noexcept -> bool {
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

  auto unwrap() noexcept -> T {
    sfc::expect(bool(_inn.ptr()), "Option::unwrap: None()");
    return static_cast<T&&>(_inn);
  }

  auto unwrap_or(T default_val) noexcept -> T {
    return _inn.ptr() ? static_cast<T&&>(_inn) : static_cast<T&&>(default_val);
  }

  auto unwrap_unchecked() noexcept -> T {
    return static_cast<T&&>(_inn);
  }

  auto expect(const auto& msg) && noexcept -> T {
    sfc::expect(bool(_inn.ptr()), "Option::expect: {}", msg);
    return static_cast<T&&>(_inn);
  }

  template <class U>
  auto operator&(Option<U> optb) && noexcept -> Option<U> {
    if (!_inn.ptr()) {
      return {};
    }
    return static_cast<Option<U>&&>(optb);
  }

  auto operator|(Option<T> optb) && noexcept -> Option<T> {
    if (_inn.ptr()) {
      return static_cast<Option<T>&&>(*this);
    }
    return static_cast<Option<T>&&>(optb);
  }

  template <class F>
  auto and_then(F&& op) && noexcept -> ops::invoke_t<F(T)> {
    if (_inn.ptr()) {
      return op(static_cast<T&&>(_inn));
    }
    return {};
  }

  auto or_else(auto&& f) && noexcept -> Option<T> {
    if (_inn.ptr()) {
      return static_cast<Option<T>&&>(*this);
    }
    return f();
  }

  template <class F>
  auto map(F&& f) && -> Option<ops::invoke_t<F(T)>> {
    if (_inn.ptr()) {
      return {f(static_cast<T&&>(_inn))};
    }
    return {};
  }

  template <class U>
  auto map_or(U default_val, auto&& f) && -> U {
    if (_inn.ptr()) {
      return f(static_cast<T&&>(_inn));
    }
    return static_cast<U&&>(default_val);
  }

 public:
  // to result
  template <class E>
  auto ok_or(E err) && noexcept -> result::Result<T, E> {
    if (_inn.ptr()) {
      return {static_cast<T&&>(_inn)};
    }
    return {static_cast<E&&>(err)};
  }

 public:
  // trait: ops::Eq
  auto operator==(const Option& other) const noexcept -> bool {
    if (!_inn.ptr()) return !other;
    if (!other) return false;
    return (_inn == *other);
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

  auto operator->() const -> const T* {
    sfc::expect(_ptr != nullptr, "Option<T&>::deref: nullptr");
    return _ptr;
  }

  auto operator->() -> T* {
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
    if (_ptr) {
      return static_cast<Option<U>&&>(optb);
    }
    return {};
  }

  auto operator|(Option<T> optb) const noexcept -> Option<T> {
    if (_ptr) {
      return *this;
    }
    return static_cast<Option<T>&&>(optb);
  }

  template <class F>
  auto and_then(F&& op) noexcept -> ops::invoke_t<F(T)> {
    if (_ptr) {
      return op(*_ptr);
    }
    return {};
  }

  auto or_else(auto&& f) noexcept -> Option<T> {
    if (_ptr) {
      return *this;
    }
    return f();
  }

  template <class F>
  auto map(F&& f) -> Option<ops::invoke_t<F(T)>> {
    if (_ptr) {
      return {f(*_ptr)};
    }
    return {};
  }

  template <class U>
  auto map_or(U default_val, auto&& f) -> U {
    if (_ptr) {
      return f(*_ptr);
    }
    return static_cast<U&&>(default_val);
  }

 public:
  // to result
  template <class E>
  auto ok_or(E err) noexcept -> result::Result<T, E> {
    if (_ptr) {
      return {*_ptr};
    }
    return {static_cast<E&&>(err)};
  }

 public:
  // trait: ops::Eq
  auto operator==(const Option& other) const noexcept -> bool {
    return _ptr == other._ptr;
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
