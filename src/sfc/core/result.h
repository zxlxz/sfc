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

  bool _tag = false;
  union {
    T _0;
    E _1;
  };

 public:
  constexpr explicit Result(T t) noexcept : _tag{true}, _0{static_cast<T&&>(t)} {}
  constexpr explicit Result(E e) noexcept : _tag{false}, _1{static_cast<E&&>(e)} {}

  constexpr Result(Ok t) noexcept : _tag{true}, _0{static_cast<T&&>(t._0)} {}
  constexpr Result(Err e) noexcept : _tag{false}, _1{static_cast<E&&>(e._0)} {}

  ~Result() noexcept {
    _tag ? mem::drop(_0) : mem::drop(_1);
  }

  Result(Result&& other) noexcept : _tag{other._tag} {
    _tag ? ptr::write(&_0, static_cast<T&&>(other._0))
         : ptr::write(&_1, static_cast<E&&>(other._1));
  }

  Result(const Result& other) noexcept : _tag{other._tag} {
    _tag ? ptr::write(&_0, other._0) : ptr::write(&_1, other._1);
  }

  Result& operator=(Result&& other) noexcept {
    if (this != &other) {
      _tag ? mem::drop(_0) : mem::drop(_1);
      _tag = other._tag;
      _tag ? ptr::write(&_0, static_cast<T&&>(other._0))
           : ptr::write(&_1, static_cast<E&&>(other._1));
    }
    return *this;
  }

  Result& operator=(const Result& other) noexcept {
    if (this != &other) {
      _tag ? mem::drop(_0) : mem::drop(_1);
      _tag = other._tag;
      _tag ? ptr::write(&_0, other._0) : ptr::write(&_1, other._1);
    }
    return *this;
  }

  constexpr auto is_ok() const noexcept -> bool {
    return _tag;
  }

  constexpr auto is_err() const noexcept -> bool {
    return !_tag;
  }

  auto unwrap_unchecked() noexcept -> T {
    return static_cast<T&&>(_0);
  }

  auto unwrap_err_unchecked() noexcept -> E {
    return static_cast<E&&>(_1);
  }

  auto unwrap(this auto self) noexcept -> T {
    sfc::expect(self._tag, "Result::unwrap: not Ok()");
    return static_cast<T&&>(self._0);
  }

  auto unwrap_err(this auto self) noexcept -> E {
    sfc::expect(!self._tag, "Result::unwrap_err: not Err()");
    return static_cast<E&&>(self._1);
  }

  auto unwrap_or(this auto self, T default_val) noexcept -> T {
    if (self._tag) return static_cast<T&&>(self._0);
    return static_cast<T&&>(default_val);
  }

  auto expect(this auto self, const auto& msg) -> T {
    sfc::expect(self._tag, "{}: {}", msg, self._1);
    return static_cast<T&&>(self._0);
  }

  auto ok(this auto self) noexcept -> Option<T> {
    if (!self._tag) return {};
    return static_cast<T&&>(self._0);
  }

  auto err(this auto self) noexcept -> Option<E> {
    if (self._tag) return {};
    return static_cast<E&&>(self._1);
  }

  template <class U>
  auto operator&(this auto self, Result<U, E> res) -> Result<U, E> {
    if (self._tag) return static_cast<Result<U, E>&&>(res);
    return Err{static_cast<E&&>(self._1)};
  }

  template <class F>
  auto operator|(this auto self, Result<T, F> res) -> Result<T, F> {
    if (self._tag) return Ok{static_cast<T&&>(self._0)};
    return static_cast<Result<T, F>&&>(res);
  }

  template <class F, class ResultUE = ops::invoke_t<F(T)>>
  auto and_then(this auto self, F&& op) -> ResultUE {
    if (self._tag) return op(static_cast<T&&>(self._0));
    return ResultUE{static_cast<E&&>(self._1)};
  }

  template <class O, class ResultTF = ops::invoke_t<O()>>
  auto or_else(this auto self, O&& op) -> ResultTF {
    if (self._tag) return ResultTF{static_cast<T&&>(self._0)};
    return op();
  }

  template <class F, class U = ops::invoke_t<F(T)>>
  auto map(this auto self, F&& op) -> Result<U, E> {
    if (self._tag) return Result<U, E>{op(static_cast<T&&>(self._0))};
    return Result<U, E>{static_cast<E&&>(self._1)};
  }

  template <class O, class F = ops::invoke_t<O(E)>>
  auto map_err(this auto self, O&& op) -> Result<T, F> {
    if (self._tag) return Result<T, F>{static_cast<T&&>(self._0)};
    return Result<T, F>{op(static_cast<E&&>(self._1))};
  }

 public:
  // trait:: ops::Eq
  auto operator==(const Result& other) const noexcept -> bool {
    if (_tag) {
      return other._tag && _0 == other._0;
    } else {
      return !other._tag && _1 == other._1;
    }
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    if (_tag) {
      f.write_fmt("Ok({})", _0);
    } else {
      f.write_fmt("Err({})", _1);
    }
  }
};

template <class E>
class [[nodiscard]] Result<void, E> {
  using Ok = result::Ok<void>;
  using Err = result::Err<E>;

  bool _tag;
  E _1;

 public:
  constexpr Result(Ok) noexcept : _tag{true}, _1{} {}
  constexpr Result(Err e) noexcept : _tag{false}, _1{static_cast<E&&>(e._0)} {}

  constexpr auto is_ok() const noexcept -> bool {
    return _tag;
  }

  constexpr auto is_err() const noexcept -> bool {
    return !_tag;
  }

  void unwrap_unchecked() const noexcept {}

  auto unwrap_err_unchecked() -> E {
    return static_cast<E&&>(_1);
  }

  void unwrap() const noexcept {
    sfc::expect(_tag, "Result::unwrap: not Ok()");
  }

  auto unwrap_err(this auto self) -> E {
    sfc::expect(!self._tag, "Result::unwrap_err: not Err()");
    return self._1;
  }

  auto err(this auto self) -> Option<E> {
    if (self._tag) return {};
    return self._1;
  }

 public:
  // trait: ops::Eq
  auto operator==(const Result& other) const noexcept -> bool {
    return _tag == other._tag;
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    if (_tag) {
      f.write_fmt("Ok()");
    } else {
      f.write_fmt("Err({})", _1);
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
