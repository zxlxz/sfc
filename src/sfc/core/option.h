#pragma once

#include "sfc/core/panicking.h"
#include "sfc/core/ptr.h"
#include "sfc/core/trait.h"

namespace sfc::option {

namespace detail {

template <class T>
class Option {
  union Data {
    T    _val;
    char _nil = 0;

    ~Data() {}
  };

  bool _tag{false};
  Data _dat{};

 public:
  inline Option() noexcept = default;

  inline explicit Option(T&& val) noexcept : _tag{true} {
    new (&_dat, mem::inplace_t{}) T{static_cast<T&&>(val)};
  }

  inline ~Option() {
    if (_tag) {
      _dat._val.~T();
    }
  }

  inline Option(const Option& other) : _tag{other._tag} {
    if (_tag) {
      new (&_dat, mem::inplace_t{}) T{other._dat._val};
    }
  }

  inline Option(Option&& other) noexcept : _tag{other._tag} {
    if (_tag) {
      new (&_dat, mem::inplace_t{}) T{static_cast<T&&>(other._dat._val)};
    }
  }

  inline auto operator=(const Option& other) noexcept -> Option& {
    if (_tag == other._tag) {
      _tag ? _dat._val = other._dat._val : void();
    } else {
      _tag ? _dat._val.~T() : new (&_dat, mem::inplace_t{}) T{other._dat._val};
      _tag = other._tag;
    }

    return *this;
  }

  inline auto operator=(Option&& other) noexcept -> Option& {
    if (_tag == other._tag) {
      _tag ? _dat._val = static_cast<T&&>(other._dat._val) : void();
    } else {
      _tag ? _dat._val.~T() : new (&_dat, mem::inplace_t{}) T{static_cast<T&&>(other._dat._val)};
      _tag = other._tag;
    }
    return *this;
  }

  inline explicit operator bool() const {
    return _tag;
  }

  inline auto get_unchecked() const -> const T& {
    return _dat._val;
  }

  inline auto get_unchecked_mut() -> T& {
    return _dat._val;
  }

  inline auto unwrap_unchecked() -> T&& {
    return static_cast<T&&>(_dat._val);
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

  auto get_unchecked() const -> const T& {
    return *_ptr;
  }

  auto get_unchecked_mut() -> T& {
    return *_ptr;
  }

  auto unwrap_unchecked() -> T& {
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
    return static_cast<bool>(_inn);
  }

  auto get_unchecked() const -> const T& {
    return _inn.get_unchecked();
  }

  auto get_unchecked_mut() -> T& {
    return _inn.get_unchecked_mut();
  }

  auto operator*() const -> const T& {
    panicking::assert(_inn, "Option::operator*: None.");
    return _inn.get_unchecked();
  }

  auto operator*() -> T& {
    panicking::assert(_inn, "Option::operator*: None.");
    return _inn.get_unchecked_mut();
  }

  auto unwrap() && -> T {
    panicking::assert(_inn, "Option::unwrap: None.");
    return static_cast<T&&>(_inn.get_unchecked_mut());
  }

  auto unwrap() const& -> T {
    panicking::assert(_inn, "Option::unwrap: None.");
    return _inn.get_unchecked();
  }

  auto unwrap_or(T default_val) && -> T {
    return _inn ? _inn.unwrap_unchecked() : default_val;
  }

  auto unwrap_or(T default_val) const& -> T {
    return _inn ? _inn.get_unchecked() : default_val;
  }

  auto expect(const auto&... msg) && -> T {
    if (!_inn) {
      panicking::panic(msg...);
    }
    return _inn.unwrap_unchecked();
  }

  auto operator==(const Option& other) const -> bool {
    if (bool(_inn) != bool(other)) {
      return false;
    }
    return _inn ? _inn.get_unchecked() == other.get_unchecked() : true;
  }

  auto map(auto&& pred) && {
    using U = decltype(pred(declval<T>()));
    return _inn ? Option<U>{pred(_inn.unwrap_unchecked())} : Option<U>{};
  }

  auto map(auto&& pred) const {
    using U = decltype(pred(declval<const T&>()));
    return _inn ? Option<U>{pred(_inn.get_unchecked())} : Option<U>{};
  }

  void fmt(auto& f) const {
    if (!_inn) {
      f.write_str("None()");
    } else {
      f.write_fmt("Some({})", _inn.get_unchecked());
    }
  }
};

}  // namespace sfc::option

namespace sfc {
using option::Option;
}  // namespace sfc
