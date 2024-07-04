#pragma once

#include "sfc/core/option.h"

namespace sfc::result {

enum class Tag : u8 { Ok, Err };

namespace detail {

template <class T, class E>
class Result {
  Tag _tag;
  union {
    T _ok;
    E _err;
  };

 public:
  explicit Result(T&& ok) : _tag{Tag::Ok}, _ok{static_cast<T&&>(ok)} {}

  explicit Result(E&& err) : _tag{Tag::Err}, _err{static_cast<E&&>(err)} {}

  ~Result() {
    switch (_tag) {
      case Tag::Ok:  _ok.~T(); break;
      case Tag::Err: _err.~E(); break;
    }
  }

  Result(const Result& other) : _tag{other._tag} {
    switch (_tag) {
      case Tag::Ok:  new (&_ok) T{other._ok}; break;
      case Tag::Err: new (&_err) E{other._err}; break;
    }
  }

  Result(Result&& other) noexcept : _tag{other._tag} {
    switch (_tag) {
      case Tag::Ok:  new (&_ok) T{static_cast<T&&>(other._ok)}; break;
      case Tag::Err: new (&_err) E{static_cast<E&&>(other._err)}; break;
    }
  }

  Result& operator=(const Result& other) {
    if (this == &other) {
      return *this;
    }
    switch (_tag) {
      case Tag::Ok:  _ok.~T(); break;
      case Tag::Err: _err.~E(); break;
    }
    switch (other._tag) {
      case Tag::Ok:  new (&_ok) T{other._ok}; break;
      case Tag::Err: new (&_err) E{other._err}; break;
    }
    _tag = other._tag;
    return *this;
  }

  Result& operator=(Result&& other) noexcept {
    if (this == &other) {
      return *this;
    }
    switch (_tag) {
      case Tag::Ok:  _ok.~T(); break;
      case Tag::Err: _err.~E(); break;
    }
    switch (other._tag) {
      case Tag::Ok:  new (&_ok) T{static_cast<T&&>(other._ok)}; break;
      case Tag::Err: new (&_err) E{static_cast<E&&>(other._err)}; break;
    }
    _tag = other._tag;
    return *this;
  }

  auto tag() const -> Tag {
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

  auto is_ok() const noexcept -> bool {
    return _inn.tag() == Tag::Ok;
  }

  auto is_err() const noexcept -> bool {
    return _inn.tag() == Tag::Err;
  }

  auto operator*() const -> const T& {
    panicking::assert(_inn.tag() == Tag::Ok, "Result::operator*: Err");
    return *_inn;
  }

  auto operator*() -> T& {
    panicking::assert(_inn.tag() == Tag::Ok, "Result::operator*: Err");
    return *_inn;
  }

  auto operator~() const -> const E& {
    panicking::assert(_inn.tag() == Tag::Err, "Result::operator*: Ok");
    return ~_inn;
  }

  auto operator~() -> E& {
    panicking::assert(_inn.tag() == Tag::Err, "Result::operator*: Ok");
    return ~_inn;
  }

  auto ok() && -> option::Option<T> {
    if (_inn.tag() == Tag::Err) {
      return {};
    }
    return static_cast<T&&>(*_inn);
  }

  auto err() && -> option::Option<E> {
    if (_inn.tag() == Tag::Ok) {
      return {};
    }
    return static_cast<E&&>(~_inn);
  }

  auto unwrap() && -> T {
    panicking::assert(_inn.tag() == Tag::Ok, "Result::unwrap: Err({})", ~_inn);
    return static_cast<T&&>(*_inn);
  }

  auto unwrap_err() && -> E {
    panicking::assert(_inn.tag() == Tag::Err, "Result::unwrap_err: Ok({})", *_inn);
    return static_cast<E&&>(~_inn);
  }

  void fmt(auto& f) const {
    if (_inn.tag() == Tag::Ok) {
      f.write_fmt("Ok({})", *_inn);
    } else {
      f.write_fmt("Err({})", ~_inn);
    }
  }
};

}  // namespace sfc::result
