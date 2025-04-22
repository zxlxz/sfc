#pragma once

#include "sfc/core/panicking.h"
#include "sfc/core/ptr.h"
#include "sfc/core/trait.h"

namespace sfc::option {

namespace detail {

template <class T>
class Option {
  bool _tag{false};
  alignas(T) u8 _buf[sizeof(T)];

 public:
  inline Option() noexcept = default;

  inline explicit Option(T&& val) noexcept : _tag{true} {
    ptr::write(reinterpret_cast<T*>(_buf), static_cast<T&&>(val));
  }

  inline ~Option() {
    if (_tag) {
      mem::drop(this->get_unchecked_mut());
    }
  }

  inline Option(const Option& other) : _tag{other._tag} {
    if (_tag) {
      ptr::write(reinterpret_cast<T*>(_buf), other.get_unchecked());
    }
  }

  inline Option(Option&& other) noexcept : _tag{other._tag} {
    if (_tag) {
      ptr::write(reinterpret_cast<T*>(_buf), other.unwrap_unchecked());
    }
  }

  inline auto operator=(const Option& other) noexcept -> Option& {
    if (_tag == other._tag) {
      _tag ? this->get_unchecked_mut() = other.get_unchecked() : void();
    } else {
      _tag ? mem::drop(this->get_unchecked_mut())
           : ptr::write(reinterpret_cast<T*>(_buf), other.get_unchecked());
      _tag = other._tag;
    }

    return *this;
  }

  inline auto operator=(Option&& other) noexcept -> Option& {
    if (_tag == other._tag) {
      _tag ? void(this->get_unchecked_mut() = other.unwrap_unchecked()) : void();
    } else {
      _tag ? mem::drop(this->get_unchecked_mut())
           : ptr::write(reinterpret_cast<T*>(_buf), other.unwrap_unchecked());
      _tag = other._tag;
    }
    return *this;
  }

  inline explicit operator bool() const {
    return _tag;
  }

  inline auto get_unchecked() const -> const T& {
    return *reinterpret_cast<const T*>(_buf);
  }

  inline auto get_unchecked_mut() -> T& {
    return *reinterpret_cast<T*>(_buf);
  }

  inline auto unwrap_unchecked() -> T&& {
    return static_cast<T&&>(*reinterpret_cast<T*>(_buf));
  }
};

template <class T>
class Option<T&> {
  T* _ptr{nullptr};

 public:
  inline Option() = default;

  inline explicit Option(T& val) noexcept : _ptr{&val} {}

  inline ~Option() = default;

  inline explicit operator bool() const {
    return _ptr != nullptr;
  }

  inline auto get_unchecked() const -> const T& {
    return *_ptr;
  }

  inline auto get_unchecked_mut() -> T& {
    return *_ptr;
  }

  inline auto unwrap_unchecked() -> T& {
    return *_ptr;
  }
};

template <class T>
  requires(requires { &T::operator bool; })
class Option<T> {
  T _val{};

 public:
  inline Option() = default;

  inline explicit Option(T&& val) noexcept : _val{static_cast<T&&>(val)} {}

  inline explicit operator bool() const {
    return static_cast<bool>(_val);
  }

  inline auto get_unchecked() const -> const T& {
    return _val;
  }

  inline auto get_unchecked_mut() -> T& {
    return _val;
  }

  inline auto unwrap_unchecked() -> T&& {
    return static_cast<T&&>(_val);
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
    panicking::assert_fmt(_inn, "Option::operator*: None.");
    return _inn.get_unchecked();
  }

  auto operator*() -> T& {
    panicking::assert_fmt(_inn, "Option::operator*: None.");
    return _inn.get_unchecked_mut();
  }

  auto unwrap() && -> T {
    panicking::assert_fmt(_inn, "Option::unwrap: None.");
    return static_cast<T&&>(_inn.get_unchecked_mut());
  }

  auto unwrap() const& -> T {
    panicking::assert_fmt(_inn, "Option::unwrap: None.");
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
      panicking::panic_fmt(msg...);
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
