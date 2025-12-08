#pragma once

#include "sfc/core/option.h"

namespace sfc::result {

enum class Tag : u8 { Ok, Err };

template <class T, class E>
struct Inner {
  T _ok = {};
  E _err = {};

 public:
  constexpr Inner(T&& ok) noexcept : _ok{static_cast<T&&>(ok)} {}

  constexpr Inner(E&& err) noexcept : _err{static_cast<E&&>(err)} {}

  constexpr auto is_ok() const -> bool {
    return _err == E{};
  }

  constexpr auto is_err() const -> bool {
    return _err != E{};
  }

  constexpr auto operator*() const -> const T& {
    return _ok;
  }

  constexpr auto operator*() -> T& {
    return _ok;
  }

  constexpr auto operator~() const -> const E& {
    return _err;
  }

  constexpr auto operator~() -> E& {
    return _err;
  }
};

template <class E>
struct Inner<void, E> {
  static_assert(trait::tv_copy_<E>);
  E _err;

 public:
  constexpr Inner() noexcept : _err{} {}

  constexpr Inner(E err) noexcept : _err{err} {}

  constexpr auto is_ok() const -> bool {
    return _err == E{};
  }

  constexpr auto is_err() const -> bool {
    return _err != E{};
  }

  constexpr auto operator~() const -> const E& {
    return _err;
  }

  constexpr auto operator~() -> E& {
    return static_cast<E&>(_err);
  }
};

template <class T, class E>
class [[nodiscard]] Result {
  Inner<T, E> _inn;

 public:
  constexpr Result(T val) noexcept : _inn{static_cast<T&&>(val)} {}
  constexpr Result(E val) noexcept : _inn{static_cast<E&&>(val)} {}

  constexpr auto is_ok() const noexcept -> bool {
    return _inn.is_ok();
  }

  constexpr auto is_err() const noexcept -> bool {
    return _inn.is_err();
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
  auto and_then(F&& op) && -> ops::invoke_t<F(T)> {
    if (_inn.is_ok()) {
      return op(static_cast<T&&>(*_inn));
    }
    return static_cast<E&&>(~_inn);
  }

  template <class O>
  auto or_else(O&& op) && -> ops::invoke_t<O()> {
    if (_inn.is_err()) {
      return op();
    }
    return static_cast<T&&>(*_inn);
  }

  template <class F, class U = ops::invoke_t<F(T)>>
  auto map(F&& op) && -> Result<U, E> {
    if (_inn.is_ok()) {
      if constexpr (trait::same_<U, void>) {
        op(static_cast<T&&>(*_inn));
        return Result<U, E>{};
      } else {
        return Result<U, E>{op(static_cast<T&&>(*_inn))};
      }
    }
    return Result<U, E>{static_cast<E&&>(~_inn)};
  }

  template <class O>
  auto map_err(O&& op) && -> Result<T, ops::invoke_t<O(E)>> {
    if (_inn.is_err()) {
      return op(static_cast<E&&>(~_inn));
    }
    return static_cast<T&&>(*_inn);
  }

 public:
  // trait: ops::Eq
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
  constexpr Result() noexcept : _inn{} {}

  constexpr Result(E val) noexcept : _inn{static_cast<E&&>(val)} {}

  constexpr auto is_ok() const noexcept -> bool {
    return _inn.is_ok();
  }

  constexpr auto is_err() const noexcept -> bool {
    return _inn.is_err();
  }

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

  template <class F>
  auto map(F&& op) const -> Result<ops::invoke_t<F()>, E> {
    if (_inn.is_ok()) {
      return op();
    }
    return {};
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

namespace sfc::option {

template <class T>
template <class E>
auto Option<T>::ok_or(E err) && -> result::Result<T, E> {
  if (_inn.is_some()) {
    return result::Result<T, E>{static_cast<T&&>(*_inn)};
  }
  return result::Result<T, E>{static_cast<E&&>(err)};
}

}  // namespace sfc::option

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
