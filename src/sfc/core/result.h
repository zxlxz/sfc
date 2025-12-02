#pragma once

#include "sfc/core/option.h"

namespace sfc::result {

enum class Tag : u8 { Ok, Err };

template <class T, class E>
struct Inner {
  T _ok = {};
  E _err = {};

 public:
  [[gnu::always_inline]] explicit Inner(T&& ok) noexcept : _ok{static_cast<T&&>(ok)} {}

  [[gnu::always_inline]] explicit Inner(E&& err) noexcept : _err{static_cast<E&&>(err)} {}

  [[gnu::always_inline]] auto is_ok() const -> bool {
    return _err == E{};
  }

  [[gnu::always_inline]] auto is_err() const -> bool {
    return _err != E{};
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
  static_assert(trait::tv_copy_<E>);
  E _err;

 public:
  [[gnu::always_inline]] explicit Inner() noexcept : _err{} {}

  [[gnu::always_inline]] explicit Inner(E err) noexcept : _err{err} {}

  [[gnu::always_inline]] auto is_ok() const -> bool {
    return _err == E{};
  }

  [[gnu::always_inline]] auto is_err() const -> bool {
    return _err != E{};
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
  Inner<T, E> _inn;

 public:
  Result(T val) noexcept : _inn{static_cast<T&&>(val)} {}
  Result(E val) noexcept : _inn{static_cast<E&&>(val)} {}

  explicit operator bool() const noexcept {
    return _inn.is_ok();
  }

  auto operator->() const noexcept -> const T* {
    panicking::expect(_inn.is_ok(), "Result::operator->: not Ok()");
    return &*_inn;
  }

  auto operator->() noexcept -> T* {
    panicking::expect(_inn.is_ok(), "Result::operator->: not Ok()");
    return &*_inn;
  }

  auto operator*() const noexcept -> const T& {
    panicking::expect(_inn.is_ok(), "Result::operator*: not Ok()");
    return *_inn;
  }

  auto operator*() noexcept -> T& {
    panicking::expect(_inn.is_ok(), "Result::operator*: not Ok()");
    return *_inn;
  }

  auto operator~() const noexcept -> const E& {
    panicking::expect(_inn.is_err(), "Result::operator~: not Err()");
    return ~_inn;
  }

  auto operator~() noexcept -> E& {
    panicking::expect(_inn.is_err(), "Result::operator~: not Err()");
    return ~_inn;
  }

  auto is_ok() const noexcept -> bool {
    return _inn.is_ok();
  }

  auto is_err() const noexcept -> bool {
    return _inn.is_err();
  }

  auto ok() && noexcept -> Option<T> {
    if (_inn.is_err()) {
      return {};
    }
    return Option<T>{static_cast<T&&>(*_inn)};
  }

  auto err() && noexcept -> Option<E> {
    if (_inn.is_ok()) {
      return {};
    }
    return static_cast<E&&>(~_inn);
  }

  auto unwrap() && noexcept -> T {
    panicking::expect(_inn.is_ok(), "Result::unwrap: not Ok()");
    return static_cast<T&&>(*_inn);
  }

  auto unwrap_or(T def) && noexcept -> T {
    if (_inn.is_ok()) {
      return static_cast<T&&>(*_inn);
    }
    return static_cast<T&&>(def);
  }

  auto unwrap_err() && noexcept -> E {
    panicking::expect(_inn.is_err(), "Result::unwrap_err: not Err()");
    return static_cast<E&&>(~_inn);
  }

  template <class U>
  auto operator&(Result<U, E> res) && noexcept -> Result<U, E> {
    if (_inn.is_ok()) {
      return static_cast<Result<U, E>&&>(res);
    }
    return ~_inn;
  }

  template <class F>
  auto operator|(Result<T, F> res) && noexcept -> Result<T, F> {
    if (_inn.is_err()) {
      return static_cast<Result<T, F>&&>(res);
    }
    return static_cast<T&&>(*_inn);
  }

  template <class F>
  auto and_then(F&& op) && -> trait::invoke_t<F(T)> {
    if (_inn.is_ok()) {
      return op(static_cast<T&&>(*_inn));
    }
    return static_cast<E&&>(~_inn);
  }

  template <class O>
  auto or_else(O&& op) && -> trait::invoke_t<O()> {
    if (_inn.is_err()) {
      return op();
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
  template <class U>
  auto operator==(const Result<U, E>& other) const -> bool {
    if (this->is_ok()) {
      return other.is_ok() && *_inn == *other._inn;
    }
    return other.is_err() && ~_inn == ~other._inn;
  }

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
  Inner<void, E> _inn;

 public:
  Result() noexcept : _inn{} {}
  Result(E val) noexcept : _inn{static_cast<E&&>(val)} {}

  void operator*() const noexcept {
    panicking::expect(_inn.is_ok(), "Result::operator~: not Ok()");
  }

  auto operator~() const noexcept -> const E& {
    panicking::expect(_inn.is_err(), "Result::operator~: not Err()");
    return ~_inn;
  }

  auto operator~() noexcept -> E& {
    panicking::expect(_inn.is_err(), "Result::operator~: not Err()");
    return ~_inn;
  }

  auto is_ok() const noexcept -> bool {
    return _inn.is_ok();
  }

  auto is_err() const noexcept -> bool {
    return _inn.is_err();
  }

  auto err() && noexcept -> Option<E> {
    if (_inn.is_ok()) {
      return {};
    }
    return static_cast<E&&>(~_inn);
  }

  auto unwrap() const noexcept -> void {
    panicking::expect(_inn.is_ok(), "Result::unwrap: not Ok()");
  }

  auto unwrap_err() && noexcept -> E {
    panicking::expect(_inn.is_err(), "Result::unwrap_err: not Err()");
    return static_cast<E&&>(~_inn);
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
