#pragma once

#include "sfc/core/option.h"

namespace sfc::result {

enum class Tag : u8 { Ok, Err };

template <class T, class E>
class Inner {
  Tag _tag;
  union {
    T _ok;
    E _err;
  };

 public:
  explicit Inner(T&& ok) noexcept : _tag{Tag::Ok}, _ok{static_cast<T&&>(ok)} {}

  explicit Inner(E&& err) noexcept : _tag{Tag::Err}, _err{static_cast<E&&>(err)} {}

  ~Inner() noexcept {
    _tag == Tag::Ok ? _ok.~T() : _err.~E();
  }

  Inner(const Inner& other) noexcept : _tag{other._tag} {
    _tag == Tag::Ok ? new (&_ok) T{other._ok} : new (&_err) E{other._err};
  }

  Inner(Inner&& other) noexcept : _tag{other._tag} {
    _tag == Tag::Ok ? new (&_ok) T{static_cast<T&&>(other._ok)} : new (&_err) E{static_cast<E&&>(other._err)};
  }

  Inner& operator=(const Inner& other) noexcept {
    if (this == &other) {
      return *this;
    }
    _tag == Tag::Ok ? _ok.~T() : _err.~E();
    other._tag == Tag::Ok ? new (&_ok) T{other._ok} : new (&_err) E{other._err};
    _tag = other._tag;
    return *this;
  }

  Inner& operator=(Inner&& other) noexcept {
    if (this == &other) {
      return *this;
    }
    _tag == Tag::Ok ? _ok.~T() : _err.~E();
    other._tag == Tag::Ok ? new (&_ok) T{static_cast<T&&>(other._ok)} : new (&_err) E{static_cast<E&&>(other._err)};
    _tag = other._tag;
    return *this;
  }

  auto tag() const noexcept -> Tag {
    return _tag;
  }

  auto operator*() const noexcept -> const T& {
    return _ok;
  }

  auto operator*() noexcept -> T& {
    return _ok;
  }

  auto operator~() const noexcept -> const E& {
    return _err;
  }

  auto operator~() noexcept -> E& {
    return _err;
  }
};

template <class T, class E>
class Result {
  static_assert(!trait::same_<T, E>);
  Inner<T, E> _inn;

 public:
  Result(T val) noexcept : _inn{static_cast<T&&>(val)} {}
  Result(E val) noexcept : _inn{static_cast<E&&>(val)} {}
  ~Result() noexcept = default;

  Result(const Result&) noexcept = default;
  Result(Result&&) noexcept = default;

  Result& operator=(const Result&) noexcept = default;
  Result& operator=(Result&&) noexcept = default;

  auto is_ok() const noexcept -> bool {
    return _inn.tag() == Tag::Ok;
  }

  auto is_err() const noexcept -> bool {
    return _inn.tag() == Tag::Err;
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
    panicking::expect(_inn.tag() == Tag::Ok, "Result::unwrap: Err({})", ~_inn);
    return static_cast<T&&>(*_inn);
  }

  auto unwrap_err() && -> E {
    panicking::expect(_inn.tag() == Tag::Err, "Result::unwrap_err: Ok({})", *_inn);
    return static_cast<E&&>(~_inn);
  }

  template <class U>
  auto operator&&(Result<U, E> res) && -> Result<U, E> {
    if (_inn.tag() == Tag::Ok) {
      return static_cast<T&&>(*_inn);
    }
    return static_cast<Result<U, E>&&>(res);
  }

  template <class F>
  auto operator||(Result<T, F> res) && -> Result<T, F> {
    if (_inn.tag() == Tag::Err) {
      return static_cast<Result<T, F>&&>(res);
    }
    return static_cast<T&&>(*_inn);
  }

  template <class F>
  auto and_then(F&& op) && -> trait::expr_t<F(T)> {
    if (_inn.tag() == Tag::Ok) {
      return op(static_cast<T&&>(*_inn));
    }
    return static_cast<E&&>(~_inn);
  }

  template <class O>
  auto or_else(O&& op) && -> trait::expr_t<O(E)> {
    if (_inn.tag() == Tag::Err) {
      return op(static_cast<E&&>(~_inn));
    }
    return static_cast<T&&>(*_inn);
  }

  template <class F, class U = trait::expr_t<F(T)>>
  auto map(F&& op) && -> Result<U, E> {
    if (_inn.tag() == Tag::Ok) {
      return op(static_cast<T&&>(*_inn));
    }
    return static_cast<E&&>(~_inn);
  }

  template <class O, class F = trait::expr_t<O(E)>>
  auto map_err(O&& op) && -> Result<T, F> {
    if (_inn.tag() == Tag::Err) {
      return op(static_cast<E&&>(~_inn));
    }
    return static_cast<T&&>(*_inn);
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
