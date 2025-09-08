#pragma once

#include "sfc/core/panicking.h"

namespace sfc::str {
struct Str;
}

namespace sfc::option {

enum class Tag : u8 { None, Some };

template <class T>
class Inner {
  Tag _tag = Tag::None;
  union {
    T _val;
  };

 public:
  explicit Inner() noexcept {}

  explicit Inner(T&& val) noexcept : _tag{Tag::Some}, _val{static_cast<T&&>(val)} {}

  ~Inner() noexcept {
    if (_tag == Tag::Some) {
      _val.~T();
    }
  }

  Inner(const Inner& other) noexcept : _tag{other._tag} {
    if (_tag == Tag::Some) {
      new (&_val) T{other._val};
    }
  }

  Inner(Inner&& other) noexcept : _tag{other._tag} {
    if (_tag == Tag::Some) {
      new (&_val) T{static_cast<T&&>(other._val)};
    }
  }

  auto operator=(const Inner& other) noexcept -> Inner& {
    if (this == &other) {
      return *this;
    }
    if (_tag == Tag::Some) {
      _val.~T();
    }
    if (other._tag == Tag::Some) {
      new (&_val) T{other._val};
    }
    _tag = other._tag;
    return *this;
  }

  auto operator=(Inner&& other) noexcept -> Inner& {
    if (this == &other) {
      return *this;
    }
    if (_tag == Tag::Some) {
      _val.~T();
    }
    if (other._tag == Tag::Some) {
      new (&_val) T{static_cast<T&&>(other._val)};
    }
    _tag = other._tag;
    return *this;
  }

  auto tag() const noexcept -> Tag {
    return _tag;
  }

  auto operator*() const noexcept -> const T& {
    return _val;
  }

  auto operator*() noexcept -> T& {
    return _val;
  }
};

template <class T>
class Inner<T&> {
  T* _ptr{nullptr};

 public:
  Inner() noexcept = default;

  explicit Inner(T& val) noexcept : _ptr{&val} {}

  auto tag() const noexcept -> Tag {
    return _ptr != nullptr ? Tag::Some : Tag::None;
  }

  auto operator*() const noexcept -> const T& {
    return *_ptr;
  }

  auto operator*() noexcept -> T& {
    return *_ptr;
  }
};

template <class T>
class Option {
  Inner<T> _inn{};

 public:
  Option() noexcept = default;
  Option(T val) noexcept : _inn{static_cast<T&&>(val)} {}
  ~Option() noexcept = default;

  Option(const Option&) noexcept = default;
  Option(Option&&) noexcept = default;

  Option& operator=(Option&&) noexcept = default;
  Option& operator=(const Option&) noexcept = default;

  explicit operator bool() const noexcept {
    return _inn.tag() == Tag::Some;
  }

  auto is_some() const noexcept -> bool {
    return _inn.tag() == Tag::Some;
  }

  auto is_none() const noexcept -> bool {
    return _inn.tag() == Tag::None;
  }

  auto operator*() const -> const T& {
    panicking::assert(_inn.tag() == Tag::Some, "Option::operator*: deref None");
    return *_inn;
  }

  auto operator*() -> T& {
    panicking::assert(_inn.tag() == Tag::Some, "Option::operator*: deref None");
    return *_inn;
  }

  auto operator->() const {
    panicking::assert(_inn.tag() == Tag::Some, "Option::operator->: deref None");
    return &*_inn;
  }

  auto operator->() {
    panicking::assert(_inn.tag() == Tag::Some, "Option::operator->: deref None");
    return &*_inn;
  }

  template <class U>
  auto operator==(const Option<U>& other) const -> bool {
    if (this->is_none()) {
      return other.is_none();
    }
    return other.is_some() && *_inn == *other;
  }

  [[nodiscard]] auto unwrap() && -> T {
    panicking::assert(_inn.tag() == Tag::Some, "Option::unwrap: None");
    return static_cast<T&&>(*_inn);
  }

  [[nodiscard]] auto unwrap_or(T default_val) && -> T {
    return _inn.tag() == Tag::Some ? static_cast<T&&>(*_inn) : static_cast<T&&>(default_val);
  }

  [[nodiscard]] auto expect(const auto&... msg) && -> T {
    panicking::assert(_inn.tag() == Tag::Some, msg...);
    return static_cast<T&&>(*_inn);
  }

  template <class U>
  [[nodiscard]] auto operator&&(Option<U> optb) const -> Option<U> {
    if (_inn.tag() == Tag::Some) {
      return static_cast<Option<U>&&>(optb);
    }
    return {};
  }

  [[nodiscard]] auto operator||(Option<T> optb) && -> Option<T> {
    if (_inn.tag() == Tag::Some) {
      return static_cast<Option&&>(*this);
    }
    return static_cast<Option&&>(optb);
  }

  template <class F, class OptionU = expr_t<F(T)>>
  [[nodiscard]] auto and_then(F&& op) && -> OptionU {
    if (_inn.tag() == Tag::None) {
      return {};
    }
    return op(static_cast<T&&>(*_inn));
  }

  template <class F>
  [[nodiscard]] auto or_else(F&& f) && -> Option {
    if (_inn.tag() == Tag::Some) {
      return static_cast<T&&>(*_inn);
    }
    return f();
  }

  template <class F, class U = expr_t<F(T)>>
  [[nodiscard]] auto map(F&& op) && -> Option<U> {
    if (_inn.tag() == Tag::None) {
      return {};
    }
    return {op(static_cast<T&&>(*_inn))};
  }

  template <class U, class F>
  [[nodiscard]] auto map_or(U default_val, F&& f) && -> U {
    if (_inn.tag() == Tag::None) {
      return static_cast<U&&>(default_val);
    }
    return f(static_cast<T&&>(*_inn));
  }

  void fmt(auto& f) const {
    if (_inn.tag() == Tag::None) {
      f.write_str("None()");
    } else {
      f.write_fmt("Some({})", *_inn);
    }
  }
};

template <class T>
Option(T) -> Option<T>;

template <usize N>
Option(const char (&s)[N]) -> Option<str::Str>;

}  // namespace sfc::option
