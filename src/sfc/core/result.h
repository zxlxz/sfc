#pragma once

#include "sfc/core/option.h"

namespace sfc::result {

enum class Tag : u8 { Ok, Err };

template <class T, class E>
class Inner {
  T _ok = {};
  E _err = {};

 public:
  [[gnu::always_inline]] explicit Inner(T&& ok) noexcept : _ok{static_cast<T&&>(ok)} {}

  [[gnu::always_inline]] explicit Inner(E&& err) noexcept : _err{static_cast<E&&>(err)} {}

  [[gnu::always_inline]] auto is_ok() const -> bool {
    return _err == nullptr;
  }

  [[gnu::always_inline]] auto is_err() const -> bool {
    return _err != nullptr;
  }

  [[gnu::always_inline]] auto operator*() const -> const T& {
    return _ok;
  }

  [[gnu::always_inline]] auto operator*() -> T& {
    return _ok;
  }

  [[gnu::always_inline]] auto operator~() const -> const E& {
    return _err;
  }

  [[gnu::always_inline]] auto operator~() -> E& {
    return _err;
  }
};

template <class E>
struct Inner<void, E> {
  E _err;

 public:
  [[gnu::always_inline]] explicit Inner() noexcept : _err{} {}

  [[gnu::always_inline]] explicit Inner(E err) noexcept : _err{err} {}

  [[gnu::always_inline]] auto is_ok() const -> bool {
    return _err == nullptr;
  }

  [[gnu::always_inline]] auto is_err() const -> bool {
    return _err != nullptr;
  }

  [[gnu::always_inline]] auto operator~() const -> const E& {
    return _err;
  }

  [[gnu::always_inline]] auto operator~() -> E& {
    return _err;
  }
};

template <class T, class E>
class [[nodiscard]] Result {
  static_assert(!__is_same(T, E));
  using Inn = Inner<T, E>;

  Inn _inn;

 public:
  Result(T val) noexcept : _inn{static_cast<T&&>(val)} {}
  Result(E val) noexcept : _inn{static_cast<E&&>(val)} {}
  ~Result() noexcept = default;

  Result(Result&&) noexcept = default;
  Result(const Result&) noexcept = default;

  Result& operator=(Result&& other) noexcept = default;
  Result& operator=(const Result& other) noexcept = default;

  explicit operator bool() const noexcept {
    return _inn.is_ok();
  }

  auto operator->() const -> const T* {
    panicking::expect(_inn.is_ok(), "Result::operator->: not Ok()");
    return &*_inn;
  }

  auto operator->() -> T* {
    panicking::expect(_inn.is_ok(), "Result::operator->: not Ok()");
    return &*_inn;
  }

  auto operator*() const -> const T& {
    panicking::expect(_inn.is_ok(), "Result::operator*: not Ok()");
    return *_inn;
  }

  auto operator*() -> T& {
    panicking::expect(_inn.is_ok(), "Result::operator*: not Ok()");
    return *_inn;
  }

  auto operator~() const -> const E& {
    panicking::expect(_inn.is_err(), "Result::operator~: not Err()");
    return ~_inn;
  }

  auto operator~() -> E& {
    panicking::expect(_inn.is_err(), "Result::operator~: not Err()");
    return ~_inn;
  }

  auto is_ok() const noexcept -> bool {
    return _inn.is_ok();
  }

  auto is_err() const noexcept -> bool {
    return _inn.is_err();
  }

  auto ok(this auto self) -> Option<T> {
    if (self._inn.is_err()) {
      return {};
    }
    return Option<T>{static_cast<T&&>(*self._inn)};
  }

  auto err(this auto self) -> Option<E> {
    if (self._inn.is_ok()) {
      return {};
    }
    return static_cast<E&&>(~self._inn);
  }

  auto unwrap(this auto self) -> T {
    if (!self._inn.is_ok()) {
      throw ~self._inn;
    }
    return static_cast<T&&>(*self._inn);
  }

  auto unwrap_err(this auto self) -> E {
    if (!self._inn.is_err()) {
      throw *self._inn;
    }
    return static_cast<E&&>(~self._inn);
  }

  auto unwrap_or(this auto self, T def) -> T {
    if (self._inn.is_ok()) {
      return static_cast<T&&>(*self._inn);
    }
    return static_cast<T&&>(def);
  }

  template <class U>
  auto operator&(this auto self, Result<U, E> res) -> Result<U, E> {
    if (self._inn.is_ok()) {
      return static_cast<T&&>(*self._inn);
    }
    return static_cast<Result<U, E>&&>(res);
  }

  template <class F>
  auto operator|(this auto self, Result<T, F> res) -> Result<T, F> {
    if (self._inn.is_err()) {
      return static_cast<Result<T, F>&&>(res);
    }
    return static_cast<T&&>(*self._inn);
  }

  template <class F>
  auto and_then(F&& op) && -> trait::invoke_t<F(T)> {
    if (_inn.is_ok()) {
      return op(static_cast<T&&>(*_inn));
    }
    return static_cast<E&&>(~_inn);
  }

  template <class O>
  auto or_else(O&& op) && -> trait::invoke_t<O(E)> {
    if (_inn.is_err()) {
      return op(static_cast<E&&>(~_inn));
    }
    return static_cast<T&&>(*_inn);
  }

  template <class F>
  auto map(F&& op) && -> Result<trait::invoke_t<F(T)>, E> {
    if (_inn.is_ok()) {
      return op(static_cast<T&&>(*_inn));
    }
    return static_cast<E&&>(~_inn);
  }

  template <class O>
  auto map_err(O&& op) && -> Result<T, trait::invoke_t<O(E)>> {
    if (_inn.is_err()) {
      return op(static_cast<E&&>(~_inn));
    }
    return static_cast<T&&>(*_inn);
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

template <class E>
class [[nodiscard]] Result<void, E> {
  static_assert(!__is_same(void, E));
  struct Ok {};
  using Inn = Inner<void, E>;
  Inn _inn;

 public:
  Result() noexcept : _inn{} {}
  Result(E err) noexcept : _inn{err} {}
  ~Result() noexcept = default;

  Result(Result&&) noexcept = default;
  Result(const Result&) noexcept = default;

  Result& operator=(Result&& other) noexcept = default;
  Result& operator=(const Result& other) noexcept = default;

  void operator*() const {
    panicking::expect(_inn.is_ok(), "Result::operator*: not Ok()");
  }

  auto operator~() const -> const E& {
    panicking::expect(_inn.is_err(), "Result::operator~: not Err()");
    return ~_inn;
  }

  auto is_ok() const noexcept -> bool {
    return _inn.is_ok();
  }

  auto is_err() const noexcept -> bool {
    return _inn.is_err();
  }

  auto err() const -> Option<E> {
    if (_inn.is_ok()) {
      return {};
    }
    return ~_inn;
  }

  auto unwrap() const -> void {
    panicking::expect(_inn.is_ok(), "Result::unwrap: not Ok()");
  }

  auto unwrap_err() const -> E {
    panicking::expect(_inn.is_err(), "Result::unwrap_err: not Err()");
    return ~_inn;
  }

 public:
  // trait: fmt::Display
  void fmt(auto& f) const {
    if (_inn.is_ok()) {
      f.write_fmt("Ok()");
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
#define _TRY(expr)                                \
  ({                                              \
    auto _res = (expr);                           \
    if (_res.is_err()) {                          \
      return ~_res;                               \
    }                                             \
    static_cast<decltype(_res)&&>(_res).unwrap(); \
  })
#endif
