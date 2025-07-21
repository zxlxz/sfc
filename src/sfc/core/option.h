#pragma once

#include "sfc/core/panicking.h"

namespace sfc::option {

namespace detail {

template <class T>
class Option {
  union Data {
    T _val;
    char _nil = 0;

    ~Data() {}
  };

  bool _tag{false};
  Data _dat{};

 public:
  inline Option() noexcept = default;

  inline explicit Option(T&& val) noexcept : _tag{true} {
    new (&_dat) T{static_cast<T&&>(val)};
  }

  inline ~Option() {
    if (_tag) {
      _dat._val.~T();
    }
  }

  inline Option(const Option& other) : _tag{other._tag} {
    if (_tag) {
      new (&_dat) T{other._dat._val};
    }
  }

  inline Option(Option&& other) noexcept : _tag{other._tag} {
    if (_tag) {
      new (&_dat) T{static_cast<T&&>(other._dat._val)};
    }
  }

  inline auto operator=(const Option& other) noexcept -> Option& {
    if (this != &other) {
      if (_tag) {
        _dat._val.~T();
      }
      if (other._tag) {
        new (&_dat) T{other._dat._val};
      }
      _tag = other._tag;
    }
    return *this;
  }

  inline auto operator=(Option&& other) noexcept -> Option& {
    if (this != &other) {
      if (_tag) {
        _dat._val.~T();
      }
      if (other._tag) {
        new (&_dat) T{static_cast<T&&>(other._dat._val)};
      }
      _tag = other._tag;
    }
    return *this;
  }

  inline explicit operator bool() const {
    return _tag;
  }

  inline auto operator*() const -> const T& {
    return _dat._val;
  }

  inline auto operator*() -> T& {
    return _dat._val;
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

  auto get_unchecked() const -> const T& {
    return *_inn;
  }

  auto get_unchecked_mut() -> T& {
    return *_inn;
  }

  auto operator*() const -> const T& {
    panicking::assert(_inn, "Option::operator*: None.");
    return *_inn;
  }

  auto operator*() -> T& {
    panicking::assert(_inn, "Option::operator*: None.");
    return *_inn;
  }

  auto operator==(const Option& other) const -> bool {
    return _inn ? other._inn && *_inn == *other._inn : !other._inn;
  }

  auto unwrap() && -> T {
    panicking::assert(_inn, "Option::unwrap: None.");
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

namespace sfc {
using option::Option;
}  // namespace sfc
