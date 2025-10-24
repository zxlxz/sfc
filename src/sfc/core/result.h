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
    if (_tag == Tag::Ok) {
      new (&_ok) T{other._ok};
    } else {
      new (&_err) E{other._err};
    }
  }

  Inner(Inner&& other) noexcept : _tag{other._tag} {
    if (_tag == Tag::Ok) {
      new (&_ok) T{static_cast<T&&>(other._ok)};
    } else {
      new (&_err) E{static_cast<E&&>(other._err)};
    }
  }

  auto is_ok() const noexcept -> bool {
    return _tag == Tag::Ok;
  }

  auto is_err() const noexcept -> bool {
    return _tag == Tag::Err;
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
  static_assert(!__is_same(T, E));
  using Ok = T;
  using Err = E;
  using Inn = Inner<T, E>;

  Inn _inn;

 public:
  Result(T val = {}) noexcept : _inn{static_cast<T&&>(val)} {}
  Result(E val) noexcept : _inn{static_cast<E&&>(val)} {}
  ~Result() noexcept = default;

  Result(const Result&) noexcept = default;
  Result(Result&&) noexcept = default;

  Result& operator=(const Result& other) noexcept {
    if (this == &other) {
      return *this;
    }
    _inn.~Inn();
    new (&_inn) Inn{other._inn};
    return *this;
  }

  Result& operator=(Result&& other) noexcept {
    if (this == &other) {
      return *this;
    }
    _inn.~Inn();
    new (&_inn) Inn{static_cast<Inn&&>(other._inn)};
    return *this;
  }

  [[nodiscard]] auto operator*() const -> const T& {
    panicking::expect(_inn.is_ok(), "Result::operator*: deref Err");
    return *_inn;
  }

  [[nodiscard]] auto operator*() -> T& {
    panicking::expect(_inn.is_ok(), "Result::operator*: deref Err");
    return *_inn;
  }

  [[nodiscard]] auto operator->() const {
    panicking::expect(_inn.is_ok(), "Result::operator->: deref Err");
    return &*_inn;
  }

  [[nodiscard]] auto operator->() {
    panicking::expect(_inn.is_ok(), "Result::operator->: deref Err");
    return &*_inn;
  }

  [[nodiscard]] auto is_ok() const noexcept -> bool {
    return _inn.is_ok();
  }

  [[nodiscard]] auto is_err() const noexcept -> bool {
    return _inn.is_err();
  }

  [[nodiscard]] auto ok(this auto self) -> Option<T> {
    if (self._inn.is_err()) {
      return {};
    }
    return Option<T>{static_cast<T&&>(*self._inn)};
  }

  [[nodiscard]] auto err(this auto self) -> Option<E> {
    if (self._inn.is_ok()) {
      return {};
    }
    return static_cast<E&&>(~self._inn);
  }

  auto unwrap_unchecked() -> T {
    return static_cast<T&&>(*_inn);
  }

  auto unwrap_err_unchecked() -> E {
    return static_cast<E&&>(~_inn);
  }

  auto unwrap(this auto self) -> T {
    panicking::expect(self._inn.is_ok(), "Result::unwrap: Err({})", ~self._inn);
    return static_cast<T&&>(*self._inn);
  }

  auto unwrap_or(this auto self, T def) -> T {
    if (self._inn.is_ok()) {
      return static_cast<T&&>(*self._inn);
    }
    return static_cast<T&&>(def);
  }

  auto unwrap_err(this auto self) -> E {
    panicking::expect(self._inn.is_err(), "Result::unwrap_err: Ok({})", *self._inn);
    return static_cast<E&&>(~self._inn);
  }

  template <class U>
  auto operator&&(this auto self, Result<U, E> res) -> Result<U, E> {
    if (self._inn.is_ok()) {
      return static_cast<T&&>(*self._inn);
    }
    return static_cast<Result<U, E>&&>(res);
  }

  template <class F>
  auto operator||(this auto self, Result<T, F> res) -> Result<T, F> {
    if (self._inn.is_err()) {
      return static_cast<Result<T, F>&&>(res);
    }
    return static_cast<T&&>(*self._inn);
  }

  template <class F, class U = trait::invoke_t<F(T)>::Ok>
  auto and_then(this auto self, F&& op) -> Result<U, E> {
    if (self._inn.is_ok()) {
      return op(static_cast<T&&>(*self._inn));
    }
    return static_cast<E&&>(~self._inn);
  }

  template <class O, class F = trait::invoke_t<O(E)>::Err>
  auto or_else(this auto self, O&& op) -> Result<T, F> {
    if (self._inn.is_err()) {
      return op(static_cast<E&&>(~self._inn));
    }
    return static_cast<T&&>(*self._inn);
  }

  template <class F, class U = trait::invoke_t<F(T)>>
  auto map(this auto self, F&& op) -> Result<U, E> {
    if (self._inn.is_ok()) {
      return op(static_cast<T&&>(*self._inn));
    }
    return static_cast<E&&>(~self._inn);
  }

  template <class O, class F = trait::invoke_t<O(E)>>
  auto map_err(this auto self, O&& op) -> Result<T, F> {
    if (self._inn.is_err()) {
      return op(static_cast<E&&>(~self._inn));
    }
    return static_cast<T&&>(*self._inn);
  }

 public:
  // trait: fmt::Display
  void fmt(auto& f) const {
    if (_inn.is_ok()) {
      f.write_fmt("Ok({})", *_inn);
    } else {
      f.write_fmt("Err({})", ~_inn);
    }
  }
};

}  // namespace sfc::result

namespace sfc {
using result::Result;
}  // namespace sfc

#ifdef __INTELLISENSE__
#define _TRY(expr) expr.unwrap()
#else
#define _TRY(expr)                        \
  ({                                      \
    auto _res = (expr);                   \
    if (_res.is_err()) {                  \
      return _res.unwrap_err_unchecked(); \
    }                                     \
    _res.unwrap_unchecked();              \
  })
#endif
