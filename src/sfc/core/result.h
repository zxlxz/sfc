#pragma once

#include "sfc/core/option.h"

namespace sfc::result {

namespace detail {

template <class T, class E>
class Result {
  bool _tag;
  union {
    T _ok;
    E _err;
  };

 public:
  Result(T&& ok) : _tag{true}, _ok{static_cast<T&&>(ok)} {}

  Result(E&& err) : _tag{false}, _err{static_cast<E&&>(err)} {}

  ~Result() {
    if (_tag) {
      _ok.~T();
    } else {
      _err.~E();
    }
  }

  Result(const Result& other) : _tag{other._tag} {
    if (_tag) {
      new (&_ok) T{other._ok};
    } else {
      new (&_err) E{other._err};
    }
  }

  Result(Result&& other) noexcept : _tag{other._tag} {
    if (_tag) {
      new (&_ok) T{static_cast<T&&>(other._ok)};
    } else {
      new (&_err) E{static_cast<E&&>(other._err)};
    }
  }

  Result& operator=(const Result& other) {
    if (this == &other) {
      return *this;
    }

    if (_tag == other._tag) {
      _tag ? _ok = other._ok : _err = other._err;
    } else {
      _tag ? _ok.~T() : _err.~E();
      _tag ? new (&_err) E{other._err} : new (&_ok) T{other._ok};
      _tag = other._tag;
    }
    return *this;
  }

  Result& operator=(Result&& other) noexcept {
    if (this == &other) {
      return *this;
    }
    if (_tag == other._tag) {
      _tag ? _ok = static_cast<T&&>(other._ok) : _err = static_cast<E&&>(other._err);
    } else {
      _tag ? _ok.~T() : _err.~E();
      _tag ? new (&_err) E{static_cast<E&&>(other._err)}
           : new (&_ok) T{static_cast<T&&>(other._ok)};
      _tag = other._tag;
    }
    return *this;
  }

  explicit operator bool() const {
    return _tag;
  }

  auto operator*() const -> const T& {
    return _ok;
  }

  auto operator*() -> T& {
    return _ok;
  }

  auto operator~() const -> const E& {
    return _err;
  }

  auto operator~() -> E& {
    return _err;
  }
};

}  // namespace detail

template <class T, class E>
class Result {
  detail::Result<T, E> _inn;

 public:
  Result(T val) noexcept : _inn{static_cast<T&&>(val)} {}

  Result(E val) noexcept : _inn{static_cast<E&&>(val)} {}

  ~Result() = default;

  Result(const Result&) = default;

  Result(Result&&) noexcept = default;

  Result& operator=(const Result&) noexcept = default;

  Result& operator=(Result&&) noexcept = default;

  operator bool() const {
    return static_cast<bool>(_inn);
  }

  auto operator*() const -> const T& {
    panicking::assert(_inn, "Result::operator*: Err");
    return *_inn;
  }

  auto operator*() -> T& {
    panicking::assert(_inn, "Result::operator*: Err");
    return *_inn;
  }

  auto operator~() const -> const E& {
    panicking::assert(!_inn, "Result::operator*: Ok");
    return ~_inn;
  }

  auto operator~() -> E& {
    panicking::assert(!_inn, "Result::operator*: Ok");
    return ~_inn;
  }

  auto ok() && -> option::Option<T> {
    if (!_inn) {
      return {};
    }
    return static_cast<T&&>(*_inn);
  }

  auto err() && -> option::Option<E> {
    if (_inn) {
      return {};
    }
    return static_cast<E&&>(~_inn);
  }

  auto unwrap() && -> T {
    panicking::assert(_inn, "Result::unwrap: Err({})", ~_inn);
    return static_cast<T&&>(*_inn);
  }

  auto unwrap_err() && -> E {
    panicking::assert(!_inn, "Result::unwrap_err: Ok({})", *_inn);
    return static_cast<E&&>(~_inn);
  }

  void fmt(auto& f) const {
    _inn ? f.write_fmt("Ok({})", *_inn) : f.write_fmt("Err({})", ~_inn);
  }
};

}  // namespace sfc::result
