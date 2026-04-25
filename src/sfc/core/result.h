#pragma once

#include "sfc/core/option.h"

namespace sfc::result {

template <class T, class E>
class [[nodiscard]] Result {
  bool _tag = false;
  union {
    T _ok;
    E _err;
  };

 public:
  constexpr Result(T val) noexcept : _tag{true}, _ok{static_cast<T&&>(val)} {}

  constexpr Result(E val) noexcept : _tag{false}, _err{static_cast<E&&>(val)} {}

  ~Result() noexcept {
    _tag ? mem::drop(_ok) : mem::drop(_err);
  }

  Result(Result&& other) noexcept : _tag{other._tag} {
    _tag ? ptr::write(&_ok, static_cast<T&&>(other._ok))
         : ptr::write(&_err, static_cast<E&&>(other._err));
  }

  Result(const Result& other) noexcept : _tag{other._tag} {
    _tag ? ptr::write(&_ok, other._ok) : ptr::write(&_err, other._err);
  }

  Result& operator=(Result&& other) noexcept {
    if (this != &other) {
      _tag ? mem::drop(_ok) : mem::drop(_err);
      _tag = other._tag;
      _tag ? ptr::write(&_ok, static_cast<T&&>(other._ok))
           : ptr::write(&_err, static_cast<E&&>(other._err));
    }
    return *this;
  }

  Result& operator=(const Result& other) noexcept {
    if (this != &other) {
      _tag ? mem::drop(_ok) : mem::drop(_err);
      _tag = other._tag;
      _tag ? ptr::write(&_ok, other._ok) : ptr::write(&_err, other._err);
    }
    return *this;
  }

  constexpr auto is_ok() const noexcept -> bool {
    return _tag;
  }

  constexpr auto is_err() const noexcept -> bool {
    return !_tag;
  }

  auto ok() && noexcept -> Option<T> {
    if (!_tag) return {};
    return static_cast<T&&>(_ok);
  }

  auto err() && noexcept -> Option<E> {
    if (_tag) return {};
    return static_cast<E&&>(_err);
  }

  auto unwrap() && noexcept -> T {
    sfc::expect(_tag, fmt::Args{"Result::unwrap: not Ok()"});
    return static_cast<T&&>(_ok);
  }

  auto unwrap_err() && noexcept -> E {
    sfc::expect(!_tag, fmt::Args{"Result::unwrap_err: not Err()"});
    return static_cast<E&&>(_err);
  }

  auto unwrap_or(T default_val) && noexcept -> T {
    if (_tag) return static_cast<T&&>(_ok);
    return static_cast<T&&>(default_val);
  }

  auto unwrap_unchecked() noexcept -> T&& {
    return static_cast<T&&>(_ok);
  }

  auto unwrap_err_unchecked() noexcept -> E&& {
    return static_cast<E&&>(_err);
  }

  auto expect(const auto& msg) && noexcept -> T {
    sfc::expect(_tag, fmt::Args{"{}: {}", msg, _err});
    return static_cast<T&&>(_ok);
  }

  template <class U>
  auto operator&(Result<U, E> res) && noexcept -> Result<U, E> {
    if (_tag) {
      return static_cast<Result<U, E>&&>(res);
    }
    return static_cast<E&&>(_err);
  }

  template <class F>
  auto operator|(Result<T, F> res) && noexcept -> Result<T, F> {
    if (_tag) {
      return static_cast<T&&>(_ok);
    }
    return static_cast<Result<T, F>&&>(res);
  }

  template <class F>
  auto and_then(F&& op) && -> ops::invoke_t<F(T)> {
    if (_tag) {
      return op(static_cast<T&&>(_ok));
    }
    return static_cast<E&&>(_err);
  }

  template <class O>
  auto or_else(O&& op) && -> ops::invoke_t<O()> {
    if (_tag) {
      return static_cast<T&&>(_ok);
    }
    return op();
  }

  template <class F>
  auto map(F&& op) && -> Result<ops::invoke_t<F(T)>, E> {
    if (_tag) {
      return op(static_cast<T&&>(_ok));
    }
    return static_cast<E&&>(_err);
  }

  template <class O>
  auto map_err(O&& op) && -> Result<T, ops::invoke_t<O(E)>> {
    if (_tag) {
      return static_cast<T&&>(_ok);
    }
    return op(static_cast<E&&>(_err));
  }

 public:
  // trait:: ops::Eq
  auto operator==(const Result& other) const noexcept -> bool {
    if (_tag) {
      return other._tag && _ok == other._ok;
    } else {
      return !other._tag && _err == other._err;
    }
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    if (_tag) {
      f.write_fmt("Ok({})", _ok);
    } else {
      f.write_fmt("Err({})", _err);
    }
  }
};

template <class E>
class [[nodiscard]] Result<void, E> {
  bool _tag;
  E _err;

 public:
  constexpr Result() noexcept : _tag{true}, _err{} {}

  constexpr Result(E err) noexcept : _tag{false}, _err{err} {}

  constexpr auto is_ok() const noexcept -> bool {
    return _tag;
  }

  constexpr auto is_err() const noexcept -> bool {
    return !_tag;
  }

  auto err() const noexcept -> Option<E> {
    if (_tag) return {};
    return _err;
  }

  void unwrap() const noexcept {
    sfc::expect(_tag, fmt::Args{"Result::unwrap: not Ok()"});
  }

  auto unwrap_err() const noexcept -> E {
    sfc::expect(!_tag, fmt::Args{"Result::unwrap_err: not Err()"});
    return _err;
  }

  void unwrap_unchecked() const noexcept {}

  auto unwrap_err_unchecked() const noexcept -> E {
    return _err;
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
      f.write_fmt("Err({})", _err);
    }
  }
};

}  // namespace sfc::result

namespace sfc {
using result::Result;
}  // namespace sfc

#if !defined(__clang_analyzer__) && !defined(__INTELLISENSE__)
#if defined(__GNUC__) || defined(__clang__)
#define _TRY(expr)                        \
  ({                                      \
    auto _res = (expr);                   \
    if (_res.is_err()) {                  \
      return _res.unwrap_err_unchecked(); \
    }                                     \
    _res.unwrap_unchecked();              \
  })
#endif
#endif

#ifndef _TRY
#define _TRY(expr) expr.unwrap()
#endif
