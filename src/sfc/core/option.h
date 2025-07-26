#pragma once

#include "sfc/core/panicking.h"

namespace sfc::option {

namespace detail {

template <class T>
class Option {
  bool _tag;
  union {
    T _val;
  };

 public:
  Option() noexcept : _tag{false} {}

  explicit Option(T&& val) noexcept : _tag{true} {
    new (&_val) T{static_cast<T&&>(val)};
  }

  ~Option() {
    if (_tag) {
      _val.~T();
    }
  }

  Option(const Option& other) : _tag{other._tag} {
    if (_tag) {
      new (&_val) T{other._val};
    }
  }

  Option(Option&& other) noexcept : _tag{other._tag} {
    if (_tag) {
      new (&_val) T{static_cast<T&&>(other._val)};
    }
  }

  auto operator=(const Option& other) noexcept -> Option& {
    if (this == &other) {
      return *this;
    }
    if (_tag) {
      _val.~T();
    }
    if (other._tag) {
      new (&_val) T{other._val};
    }
    _tag = other._tag;
    return *this;
  }

  auto operator=(Option&& other) noexcept -> Option& {
    if (this != &other) {
      if (_tag) {
        _val.~T();
      }
      if (other._tag) {
        new (&_val) T{static_cast<T&&>(other._val)};
      }
      _tag = other._tag;
    }
    return *this;
  }

  explicit operator bool() const {
    return _tag;
  }

  auto operator*() const -> const T& {
    return _val;
  }

  auto operator*() -> T& {
    return _val;
  }
};

template <class T>
class Option<T&> {
  T* _ptr{nullptr};

 public:
  Option() = default;

  explicit Option(T& val) noexcept : _ptr{&val} {}

  explicit operator bool() const {
    return _ptr != nullptr;
  }

  auto operator*() const -> const T& {
    return *_ptr;
  }

  auto operator*() -> T& {
    return *_ptr;
  }
};

}  // namespace detail

template <class T>
class Option {
  detail::Option<T> _inn{};

 public:
  Option() = default;

  Option(T val) noexcept : _inn{static_cast<T&&>(val)} {}

  ~Option() = default;

  Option(const Option&) noexcept = default;

  Option(Option&&) noexcept = default;

  auto operator=(Option&&) noexcept -> Option& = default;

  operator bool() const {
    return bool(_inn);
  }

  auto operator*() const -> const T& {
    panicking::assert(_inn, "Option::operator*: deref None");
    return *_inn;
  }

  auto operator*() -> T& {
    panicking::assert(_inn, "Option::operator*: deref None");
    return *_inn;
  }

  auto operator->() const {
    panicking::assert(_inn, "Option::operator->: deref None");
    return &*_inn;
  }

  auto operator->() {
    panicking::assert(_inn, "Option::operator->: deref None");
    return &*_inn;
  }

  auto operator==(const Option& other) const -> bool {
    return _inn ? other._inn && *_inn == *other._inn : !other._inn;
  }

  auto unwrap() && -> T {
    panicking::assert(_inn, "Option::unwrap: None");
    return static_cast<T&&>(*_inn);
  }

  auto unwrap_or(T default_val) && -> T {
    return _inn ? static_cast<T&&>(*_inn) : default_val;
  }

  auto expect(const auto&... msg) && -> T {
    panicking::assert(_inn, msg...);
    return static_cast<T&&>(*_inn);
  }

  auto map(auto&& pred) && {
    using U = decltype(pred(static_cast<T&&>(*_inn)));
    return _inn ? Option<U>{pred(static_cast<T&&>(*_inn))} : Option<U>{};
  }

  void fmt(auto& f) const {
    if (!_inn) {
      f.write_str("None()");
    } else {
      f.write_fmt("Some({})", *_inn);
    }
  }
};

}  // namespace sfc::option
