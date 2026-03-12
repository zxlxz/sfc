#pragma once

#include "sfc/core/option.h"

namespace sfc::result {

enum class Tag : u8 { Err = 0, Ok = 1 };

template <class T, class E>
class [[nodiscard]] Result {
  Tag _tag;
  union {
    T _ok;
    E _err;
  };

 public:
  constexpr Result(T val) noexcept : _tag{Tag::Ok}, _ok{static_cast<T&&>(val)} {}
  constexpr Result(E val) noexcept : _tag{Tag::Err}, _err{static_cast<E&&>(val)} {}

  ~Result() noexcept {
    if (_tag == Tag::Ok) {
      mem::drop(_ok);
    } else {
      mem::drop(_err);
    }
  }

  Result(Result&& other) noexcept : _tag{other._tag} {
    _tag == Tag::Ok ? ptr::write(&_ok, static_cast<T&&>(other._ok)) : ptr::write(&_err, static_cast<E&&>(other._err));
  }

  Result(const Result& other) noexcept : _tag{other._tag} {
    _tag == Tag::Ok ? ptr::write(&_ok, other._ok) : ptr::write(&_err, other._err);
  }

  Result& operator=(Result&& other) noexcept {
    if (this != &other) {
      _tag == Tag::Ok ? mem::drop(_ok) : mem::drop(_err);
      _tag = other._tag;
      _tag == Tag::Ok ? ptr::write(&_ok, static_cast<T&&>(other._ok)) : ptr::write(&_err, static_cast<E&&>(other._err));
    }
    return *this;
  }

  Result& operator=(const Result& other) noexcept {
    if (this != &other) {
      _tag == Tag::Ok ? mem::drop(_ok) : mem::drop(_err);
      _tag = other._tag;
      _tag == Tag::Ok ? ptr::write(&_ok, other._ok) : ptr::write(&_err, other._err);
    }
    return *this;
  }

  constexpr auto is_ok() const noexcept -> bool {
    return _tag == Tag::Ok;
  }

  constexpr auto is_err() const noexcept -> bool {
    return _tag == Tag::Err;
  }

  auto ok(this auto self) noexcept -> Option<T> {
    if (self._tag == Tag::Err) return {};
    return static_cast<T&&>(self._ok);
  }

  auto err(this auto self) noexcept -> Option<E> {
    if (self._tag == Tag::Ok) return {};
    return static_cast<E&&>(self._err);
  }

  auto unwrap(this auto self) noexcept -> T {
    sfc::expect(self._tag == Tag::Ok, "Result::unwrap: not Ok()");
    return static_cast<T&&>(self._ok);
  }

  auto unwrap_err(this auto self) noexcept -> E {
    sfc::expect(self._tag == Tag::Err, "Result::unwrap_err: not Err()");
    return static_cast<E&&>(self._err);
  }

  auto unwrap_or(this auto self, T default_val) noexcept -> T {
    if (self._tag == Tag::Ok) return static_cast<T&&>(self._ok);
    return static_cast<T&&>(default_val);
  }

  auto unwrap_unchecked() noexcept -> T&& {
    return static_cast<T&&>(_ok);
  }

  auto unwrap_err_unchecked() noexcept -> E&& {
    return static_cast<E&&>(_err);
  }

  template <class U>
  auto operator&(this auto self, Result<U, E> res) noexcept -> Result<U, E> {
    if (self._tag == Tag::Ok) {
      return static_cast<Result<U, E>&&>(res);
    }
    return static_cast<E&&>(self._err);
  }

  template <class F>
  auto operator|(this auto self, Result<T, F> res) noexcept -> Result<T, F> {
    if (self._tag == Tag::Ok) {
      return static_cast<T&&>(self._ok);
    }
    return static_cast<Result<T, F>&&>(res);
  }

  template <class F>
  auto and_then(this auto self, F&& op) -> ops::invoke_t<F(T)> {
    if (self._tag == Tag::Ok) {
      return op(static_cast<T&&>(self._ok));
    }
    return static_cast<E&&>(self._err);
  }

  template <class O>
  auto or_else(this auto self, O&& op) -> ops::invoke_t<O()> {
    if (self._tag == Tag::Ok) {
      return static_cast<T&&>(self._ok);
    }
    return op();
  }

  template <class F>
  auto map(this auto self, F&& op) -> Result<ops::invoke_t<F(T)>, E> {
    if (self._tag == Tag::Ok) {
      return op(static_cast<T&&>(self._ok));
    }
    return static_cast<E&&>(self._err);
  }

  template <class O>
  auto map_err(this auto self, O&& op) -> Result<T, ops::invoke_t<O(E)>> {
    if (self._tag == Tag::Ok) {
      return static_cast<T&&>(self._ok);
    }
    return op(static_cast<E&&>(self._err));
  }

 public:
  // trait:: ops::Eq
  auto operator==(const Result& other) const noexcept -> bool {
    if (_tag == Tag::Ok) {
      return other._tag == Tag::Ok && _ok == other._ok;
    } else {
      return other._tag == Tag::Err && _err == other._err;
    }
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    if (_tag == Tag::Ok) {
      f.write_fmt("Ok({})", _ok);
    } else {
      f.write_fmt("Err({})", _err);
    }
  }
};

template <class E>
class [[nodiscard]] Result<void, E> {
  Tag _tag;
  E _err;

 public:
  constexpr Result() noexcept : _tag{Tag::Ok} {}

  constexpr Result(E err) noexcept : _tag{Tag::Err}, _err{err} {}

  constexpr auto is_ok() const noexcept -> bool {
    return _tag == Tag::Ok;
  }

  constexpr auto is_err() const noexcept -> bool {
    return _tag == Tag::Err;
  }

  auto err() const noexcept -> Option<E> {
    if (_tag == Tag::Ok) return {};
    return _err;
  }

  void unwrap() const noexcept {
    sfc::expect(_tag == Tag::Ok, "Result::unwrap: not Ok()");
  }

  auto unwrap_err() const noexcept -> E {
    sfc::expect(_tag == Tag::Err, "Result::unwrap_err: not Err()");
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
    if (_tag == Tag::Ok) {
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
