#pragma once

#include "sfc/core/option.h"

namespace sfc::result {

template <class T>
struct Ok {
  T _0;
};

template <class E>
struct Err {
  E _0;
};

template <>
struct Ok<void> {};

template <class T>
Ok(T) -> Ok<T>;

Ok() -> Ok<void>;

template <class T, class E>
class [[nodiscard]] Result {
  using Ok = result::Ok<T>;
  using Err = result::Err<E>;

  u8 _tag;
  union {
    T _0;
    E _1;
  };

 public:
  constexpr explicit Result(T t) noexcept : _tag{0}, _0{static_cast<T&&>(t)} {}
  constexpr explicit Result(E e) noexcept : _tag{1}, _1{static_cast<E&&>(e)} {}

  constexpr Result(Ok t) noexcept : _tag{0}, _0{static_cast<T&&>(t._0)} {}
  constexpr Result(Err e) noexcept : _tag{1}, _1{static_cast<E&&>(e._0)} {}

  ~Result() noexcept {
    switch (_tag) {
      case 0:  mem::drop(_0); break;
      case 1:  mem::drop(_1); break;
      default: break;
    }
  }

  Result(Result&& other) noexcept : _tag{0xFF} {
    switch (other._tag) {
      case 0:  ptr::write(&_0, static_cast<T&&>(other._0)), _tag = 0; break;
      case 1:  ptr::write(&_1, static_cast<E&&>(other._1)), _tag = 1; break;
      default: break;
    }
  }

  Result(const Result& other) noexcept : _tag{0xFF} {
    switch (other._tag) {
      case 0:  ptr::write(&_0, other._0), _tag = 0; break;
      case 1:  ptr::write(&_1, other._1), _tag = 1; break;
      default: break;
    }
  }

  Result& operator=(Result&& other) noexcept {
    if (this == &other) return *this;
    switch (_tag) {
      case 0:  mem::drop(_0), _tag = 0xFF; break;
      case 1:  mem::drop(_1), _tag = 0xFF; break;
      default: break;
    }
    switch (other._tag) {
      case 0:  ptr::write(&_0, static_cast<T&&>(other._0)), _tag = 0; break;
      case 1:  ptr::write(&_1, static_cast<E&&>(other._1)), _tag = 1; break;
      default: break;
    }
    return *this;
  }

  Result& operator=(const Result& other) noexcept {
    if (this == &other) return *this;
    switch (_tag) {
      case 0:  mem::drop(_0), _tag = 0xFF; break;
      case 1:  mem::drop(_1), _tag = 0xFF; break;
      default: break;
    }
    switch (other._tag) {
      case 0:  ptr::write(&_0, other._0), _tag = 0; break;
      case 1:  ptr::write(&_1, other._1), _tag = 1; break;
      default: break;
    }
    return *this;
  }

  constexpr auto is_ok() const noexcept -> bool {
    return _tag == 0;
  }

  constexpr auto is_err() const noexcept -> bool {
    return _tag == 1;
  }

  auto unwrap_unchecked() noexcept -> T {
    return static_cast<T&&>(_0);
  }

  auto unwrap_err_unchecked() noexcept -> E {
    return static_cast<E&&>(_1);
  }

  auto unwrap() && -> T {
    sfc::expect(_tag == 0, "called `Result::unwrap()` on Err({})", _1);
    return static_cast<T&&>(_0);
  }

  auto unwrap_err() && -> E {
    sfc::expect(_tag == 1, "called `Result::unwrap_err()` on Ok({})", _0);
    return static_cast<E&&>(_1);
  }

  auto unwrap_or(T default_val) && -> T {
    if (_tag == 0) return static_cast<T&&>(_0);
    return static_cast<T&&>(default_val);
  }

  auto expect(const auto& msg) && -> T {
    sfc::expect(_tag == 0, "{}: Err({})", msg, _1);
    return static_cast<T&&>(_0);
  }

  auto ok() && -> Option<T> {
    if (_tag != 0) return {};
    return static_cast<T&&>(_0);
  }

  auto err() && -> Option<E> {
    if (_tag != 1) return {};
    return static_cast<E&&>(_1);
  }

  template <class U>
  auto operator&(Result<U, E> res) && -> Result<U, E> {
    if (_tag == 0) return static_cast<Result<U, E>&&>(res);
    return Err{static_cast<E&&>(_1)};
  }

  template <class F>
  auto operator|(Result<T, F> res) && -> Result<T, F> {
    if (_tag == 0) return Ok{static_cast<T&&>(_0)};
    return static_cast<Result<T, F>&&>(res);
  }

  template <class F, class ResultUE = ops::invoke_t<F(T)>>
  auto and_then(F&& op) && -> ResultUE {
    if (_tag == 0) return op(static_cast<T&&>(_0));
    return ResultUE{static_cast<E&&>(_1)};
  }

  template <class O, class ResultTF = ops::invoke_t<O()>>
  auto or_else(O&& op) && -> ResultTF {
    if (_tag == 0) return ResultTF{static_cast<T&&>(_0)};
    return op();
  }

  template <class F, class U = ops::invoke_t<F(T)>>
  auto map(F&& op) && -> Result<U, E> {
    if (_tag == 0) return Result<U, E>{op(static_cast<T&&>(_0))};
    return Result<U, E>{static_cast<E&&>(_1)};
  }

  template <class O, class F = ops::invoke_t<O(E)>>
  auto map_err(O&& op) && -> Result<T, F> {
    if (_tag == 1) return Result<T, F>{op(static_cast<E&&>(_1))};
    return Result<T, F>{static_cast<T&&>(_0)};
  }

 public:
  // trait:: ops::Eq
  auto operator==(const Result& other) const noexcept -> bool {
    if (_tag == 0) {
      return other._tag == 0 && _0 == other._0;
    } else {
      return other._tag == 1 && _1 == other._1;
    }
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    switch (_tag) {
      case 0:  f.write_fmt("Ok({})", _0); break;
      case 1:  f.write_fmt("Err({})", _1); break;
      default: f.write_str("Result(?)"); break;
    }
  }
};

template <class E>
class [[nodiscard]] Result<void, E> {
  using Ok = result::Ok<void>;
  using Err = result::Err<E>;

  u8 _tag;
  union {
    E _1;
  };

 public:
  constexpr Result(Ok) noexcept : _tag{0}, _1{} {}
  constexpr Result(Err e) noexcept : _tag{1}, _1{static_cast<E&&>(e._0)} {}

  Result(const Result& other) noexcept : _tag{0xFF} {
    switch (other._tag) {
      case 0:  _tag = 0; break;
      case 1:  ptr::write(&_1, other._1), _tag = 1; break;
      default: break;
    }
  }

  Result(Result&& other) noexcept : _tag{0xFF} {
    switch (other._tag) {
      case 0:  _tag = 0; break;
      case 1:  ptr::write(&_1, static_cast<E&&>(other._1)), _tag = 1; break;
      default: break;
    }
  }

  ~Result() {
    switch (_tag) {
      case 0:  break;
      case 1:  mem::drop(_1); break;
      default: break;
    }
  }

  constexpr auto is_ok() const noexcept -> bool {
    return _tag == 0;
  }

  constexpr auto is_err() const noexcept -> bool {
    return _tag == 1;
  }

  void unwrap_unchecked() const noexcept {}

  auto unwrap_err_unchecked() -> E {
    return static_cast<E&&>(_1);
  }

  void unwrap() const noexcept {
    sfc::expect(_tag == 0, "Result::unwrap: not Ok()");
  }

  auto unwrap_err() && -> E {
    sfc::expect(_tag == 1, "Result::unwrap_err: not Err()");
    return static_cast<E&&>(_1);
  }

  auto err() && -> Option<E> {
    if (_tag != 1) return {};
    return static_cast<E&&>(_1);
  }

 public:
  // trait: ops::Eq
  auto operator==(const Result& other) const noexcept -> bool {
    return _tag == other._tag;
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    switch (_tag) {
      case 0:  f.write_str("Ok()"); break;
      case 1:  f.write_fmt("Err({})", _1); break;
      default: f.write_str("Result(?)"); break;
    }
  }
};

}  // namespace sfc::result

namespace sfc {
using result::Ok;
using result::Err;
using result::Result;
}  // namespace sfc

#if !defined(__clang_analyzer__) && !defined(__INTELLISENSE__)
#if defined(__GNUC__) || defined(__clang__)
#define _TRY(expr)                             \
  ({                                           \
    auto _res = (expr);                        \
    if (_res.is_err()) {                       \
      return Err{_res.unwrap_err_unchecked()}; \
    }                                          \
    _res.unwrap_unchecked();                   \
  })
#endif
#endif

#ifndef _TRY
#define _TRY(expr) expr.unwrap()
#endif
