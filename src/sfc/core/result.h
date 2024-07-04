#pragma once

#include "sfc/core/option.h"

namespace sfc::result {

namespace detail {

template <class T, class E>
class Result {
  bool _tag;
  union {
    T _0;
    E _1;
  };

 public:
  Result(T&& ok) : _tag{true}, _0{static_cast<T&&>(ok)} {}

  Result(E&& err) : _tag{false}, _1{static_cast<E&&>(err)} {}

  ~Result() {
    _tag ? _0.~T() : _1.~E();
  }

  Result(const Result& other) : _tag{other._tag} {
    _tag ? new (&_0) T{other._0} : new (&_1) E{other._1};
  }

  Result(Result&& other) noexcept : _tag{other._tag} {
    _tag ? new (&_0) T{static_cast<T&&>(other._0)} : new (&_1) E{static_cast<E&&>(other._1)};
  }

  Result& operator=(const Result& other) {
    if (this == &other) {
      return *this;
    }

    if (_tag == other._tag) {
      _tag ? _0 = other._0 : _1 = other._1;
    } else {
      _tag ? _0.~T() : _1.~E();
      _tag = other._tag;
      _tag ? new (&_0) T{static_cast<T&&>(other._0)} : new (&_1) E{static_cast<E&&>(other._1)};
    }
    return *this;
  }

  Result& operator=(Result&& other) noexcept {
    if (this == &other) {
      return *this;
    }
    if (_tag == other._tag) {
      _tag ? _0 = static_cast<T&&>(other._0) : _1 = static_cast<E&&>(other._1);
    } else {
      _tag ? _0.~T() : _1.~E();
      _tag = other._tag;
      _tag ? new (&_0) T{static_cast<T&&>(other._0)}
           : new (&_1) E{static_cast<E&&>(other._1)};
    }
    return *this;
  }

  explicit operator bool() const {
    return _tag;
  }

  auto operator*() const -> const T& {
    return _0;
  }

  auto operator*() -> T& {
    return _0;
  }

  auto operator~() const -> const E& {
    return _1;
  }

  auto operator~() -> E& {
    return _1;
  }
};

}  // namespace detail

template <class T, class E>
class Result {
  static_assert(!__is_same(T, E));
  detail::Result<T, E> _inn;

 public:
  Result(T val) noexcept : _inn{static_cast<T&&>(val)} {}

  Result(E val) noexcept : _inn{static_cast<E&&>(val)} {}

  ~Result() = default;

  Result(const Result&) = default;

  Result(Result&&) noexcept = default;

  Result& operator=(const Result&) noexcept = default;

  Result& operator=(Result&&) noexcept = default;

  operator bool() const noexcept {
    return static_cast<bool>(_inn);
  }

  auto is_ok() const noexcept -> bool {
    return static_cast<bool>(_inn);
  }

  auto is_err() const noexcept -> bool {
    return !static_cast<bool>(_inn);
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
    if (_inn) {
      f.write_fmt("Ok({})", *_inn);
    } else {
      f.write_fmt("Err({})", ~_inn);
    }
  }
};

}  // namespace sfc::result
