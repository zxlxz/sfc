#pragma once

#include "sfc/core/panicking.h"
#include "sfc/core/trait.h"

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

  auto is_some() const noexcept -> bool {
    return _tag == Tag::Some;
  }

  auto is_none() const noexcept -> bool {
    return _tag == Tag::None;
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

  auto is_some() const noexcept -> bool {
    return _ptr != nullptr;
  }

  auto is_none() const noexcept -> bool {
    return _ptr == nullptr;
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
  using Some = T;
  using Inn = Inner<T>;

  Inn _inn{};

 public:
  Option() noexcept = default;
  Option(T val) noexcept : _inn{static_cast<T&&>(val)} {}
  ~Option() noexcept = default;

  Option(const Option&) noexcept = default;
  Option(Option&&) noexcept = default;

  Option& operator=(const Option& other) {
    if (this == &other) {
      return *this;
    }
    _inn.~Inn();
    new (&_inn) Inn{other._inn};
    return *this;
  }

  Option& operator=(Option&& other) noexcept {
    if (this == &other) {
      return *this;
    }
    _inn.~Inn();
    new (&_inn) Inn{static_cast<Inn&&>(other._inn)};
    return *this;
  }

  auto is_some() const noexcept -> bool {
    return _inn.is_some();
  }

  auto is_none() const noexcept -> bool {
    return _inn.is_none();
  }

  explicit operator bool() const noexcept {
    return _inn.is_some();
  }

  auto operator*() const -> const T& {
    panicking::expect(_inn.is_some(), "Option::operator*: deref None");
    return *_inn;
  }

  auto operator*() -> T& {
    panicking::expect(_inn.is_some(), "Option::operator*: deref None");
    return *_inn;
  }

  auto operator->() const {
    panicking::expect(_inn.is_some(), "Option::operator->: deref None");
    return &*_inn;
  }

  auto operator->() {
    panicking::expect(_inn.is_some(), "Option::operator->: deref None");
    return &*_inn;
  }

  template <class U>
  auto operator==(const Option<U>& other) const -> bool {
    if (this->is_none()) {
      return other.is_none();
    }
    return other.is_some() && *_inn == *other;
  }

  [[nodiscard]] auto unwrap(this auto self) -> T {
    panicking::expect(self._inn.is_some(), "Option::unwrap: None");
    return static_cast<T&&>(*self._inn);
  }

  [[nodiscard]] auto unwrap_or(this auto self, T default_val) -> T {
    if (self._inn.is_some()) {
      return static_cast<T&&>(*self._inn);
    }
    return static_cast<T&&>(default_val);
  }

  [[nodiscard]] auto expect(this auto self, const auto&... msg) -> T {
    panicking::expect(self._inn.is_some(), msg...);
    return static_cast<T&&>(*self._inn);
  }

  template <class U>
  [[nodiscard]] auto operator&&(Option<U> optb) const -> Option<U> {
    if (_inn.is_some()) {
      return static_cast<Option<U>&&>(optb);
    }
    return {};
  }

  [[nodiscard]] auto operator||(this auto self, Option<T> optb) -> Option<T> {
    if (self._inn.is_some()) {
      return static_cast<Option&&>(self);
    }
    return static_cast<Option&&>(optb);
  }

  template <class F, class U = typename trait::invoke_t<F(T)>::Some>
  [[nodiscard]] auto and_then(this auto self, F&& op) -> Option<U> {
    if (self._inn.is_none()) {
      return {};
    }
    return op(static_cast<T&&>(*self._inn));
  }

  template <class F>
  [[nodiscard]] auto or_else(this auto self, F&& f) -> Option {
    if (self._inn.is_some()) {
      return static_cast<T&&>(*self._inn);
    }
    return f();
  }

  template <class F, class U = trait::invoke_t<F(T)>>
  [[nodiscard]] auto map(this auto self, F&& op) -> Option<U> {
    if (self._inn.is_none()) {
      return {};
    }
    return {op(static_cast<T&&>(*self._inn))};
  }

  template <class U>
  [[nodiscard]] auto map_or(this auto self, U default_val, auto&& f) -> U {
    if (self._inn.is_none()) {
      return static_cast<U&&>(default_val);
    }
    return f(static_cast<T&&>(*self._inn));
  }

  void fmt(auto& f) const {
    if (_inn.is_none()) {
      f.write_str("None()");
    } else {
      f.write_fmt("Some({})", *_inn);
    }
  }
};

template <class T>
Option(T) -> Option<T>;

}  // namespace sfc::option
