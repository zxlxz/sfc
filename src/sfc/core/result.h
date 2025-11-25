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
  [[gnu::always_inline]] explicit Inner(T&& ok) noexcept : _tag{Tag::Ok}, _ok{static_cast<T&&>(ok)} {}

  [[gnu::always_inline]] explicit Inner(E&& err) noexcept : _tag{Tag::Err}, _err{static_cast<E&&>(err)} {}

  [[gnu::always_inline]] ~Inner() noexcept {
    switch (_tag) {
      case Tag::Ok:  _ok.~T(); break;
      case Tag::Err: _err.~E(); break;
    }
  }

  [[gnu::always_inline]] Inner(Inner&& other) noexcept : _tag{other._tag} {
    switch (_tag) {
      case Tag::Ok:  new (&_ok) T{static_cast<T&&>(other._ok)}; break;
      case Tag::Err: new (&_err) E{static_cast<E&&>(other._err)}; break;
    }
  }

  [[gnu::always_inline]] Inner(const Inner& other) noexcept : _tag{other._tag} {
    switch (_tag) {
      case Tag::Ok:  new (&_ok) T{other._ok}; break;
      case Tag::Err: new (&_err) E{other._err}; break;
    }
  }

  [[gnu::always_inline]] Inner& operator=(Inner&& other) noexcept {
    if (this == &other) {
      return *this;
    }
    if (_tag == other._tag) {
      switch (_tag) {
        case Tag::Ok:  _ok = static_cast<T&&>(other._ok); break;
        case Tag::Err: _err = static_cast<E&&>(other._err); break;
      }
    } else {
      switch (_tag) {
        case Tag::Ok:  _ok.~T(); break;
        case Tag::Err: _err.~E(); break;
      }
      _tag = other._tag;
      switch (_tag) {
        case Tag::Ok:  new (&_ok) T{static_cast<T&&>(other._ok)}; break;
        case Tag::Err: new (&_err) E{static_cast<E&&>(other._err)}; break;
      }
    }
    return *this;
  }

  [[gnu::always_inline]] Inner& operator=(const Inner& other) {
    if (this == &other) {
      return *this;
    }
    if (_tag == other._tag) {
      switch (_tag) {
        case Tag::Ok:  _ok = other._ok; break;
        case Tag::Err: _err = other._err; break;
      }
    } else {
      switch (_tag) {
        case Tag::Ok:  _ok.~T(); break;
        case Tag::Err: _err.~E(); break;
      }
      _tag = other._tag;
      switch (_tag) {
        case Tag::Ok:  new (&_ok) T{other._ok}; break;
        case Tag::Err: new (&_err) E{other._err}; break;
      }
    }
    return *this;
  }

  [[gnu::always_inline]] auto is_ok() const -> bool {
    return _tag == Tag::Ok;
  }

  [[gnu::always_inline]] auto is_err() const -> bool {
    return _tag == Tag::Err;
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
class Inner<void, E> {
  Tag _tag;
  union {
    E _err;
  };

 public:
  [[gnu::always_inline]] explicit Inner() noexcept : _tag{Tag::Ok} {}

  [[gnu::always_inline]] explicit Inner(E&& err) noexcept : _tag{Tag::Err}, _err{static_cast<E&&>(err)} {}

  [[gnu::always_inline]] ~Inner() noexcept {
    switch (_tag) {
      case Tag::Ok:  break;
      case Tag::Err: _err.~E(); break;
    }
  }

  [[gnu::always_inline]] Inner(Inner&& other) noexcept : _tag{other._tag} {
    switch (_tag) {
      case Tag::Ok:  break;
      case Tag::Err: new (&_err) E{static_cast<E&&>(other._err)}; break;
    }
  }

  [[gnu::always_inline]] Inner(const Inner& other) noexcept : _tag{other._tag} {
    switch (_tag) {
      case Tag::Ok:  break;
      case Tag::Err: new (&_err) E{other._err}; break;
    }
  }

  [[gnu::always_inline]] Inner& operator=(Inner&& other) noexcept {
    if (this == &other) {
      return *this;
    }
    if (_tag == other._tag) {
      switch (_tag) {
        case Tag::Ok:  break;
        case Tag::Err: _err = static_cast<E&&>(other._err); break;
      }
    } else {
      switch (_tag) {
        case Tag::Ok:  break;
        case Tag::Err: _err.~E(); break;
      }
      _tag = other._tag;
      switch (_tag) {
        case Tag::Ok:  break;
        case Tag::Err: new (&_err) E{static_cast<E&&>(other._err)}; break;
      }
    }
    return *this;
  }

  [[gnu::always_inline]] Inner& operator=(const Inner& other) {
    if (this == &other) {
      return *this;
    }
    if (_tag == other._tag) {
      switch (_tag) {
        case Tag::Ok:  break;
        case Tag::Err: _err = other._err; break;
      }
    } else {
      switch (_tag) {
        case Tag::Ok:  break;
        case Tag::Err: _err.~E(); break;
      }
      _tag = other._tag;
      switch (_tag) {
        case Tag::Ok:  break;
        case Tag::Err: new (&_err) E{other._err}; break;
      }
    }
    return *this;
  }

  [[gnu::always_inline]] auto is_ok() const -> bool {
    return _tag == Tag::Ok;
  }

  [[gnu::always_inline]] auto is_err() const -> bool {
    return _tag == Tag::Err;
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

  auto ok() && -> Option<T> {
    if (_inn.is_err()) {
      return {};
    }
    return Option<T>{static_cast<T&&>(*_inn)};
  }

  auto err() && -> Option<E> {
    if (_inn.is_ok()) {
      return {};
    }
    return static_cast<E&&>(~_inn);
  }

  auto unwrap() && -> T {
    panicking::expect(_inn.is_ok(), "Result::unwrap: not Ok()");
    return static_cast<T&&>(*_inn);
  }

  auto unwrap_or(T def) && -> T {
    if (_inn.is_ok()) {
      return static_cast<T&&>(*_inn);
    }
    return static_cast<T&&>(def);
  }

  auto unwrap_err() && -> E {
    panicking::expect(_inn.is_err(), "Result::unwrap_err: not Err()");
    return static_cast<E&&>(~_inn);
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

template <class E>
class [[nodiscard]] Result<void, E> {
  static_assert(!__is_same(void, E));
  using Inn = Inner<void, E>;

  Inn _inn;

 public:
  Result() noexcept : _inn{} {}
  Result(E val) noexcept : _inn{static_cast<E&&>(val)} {}
  ~Result() noexcept = default;

  Result(Result&&) noexcept = default;
  Result(const Result&) noexcept = default;

  Result& operator=(Result&& other) noexcept = default;
  Result& operator=(const Result& other) noexcept = default;

  void operator*() const {
    panicking::expect(_inn.is_ok(), "Result::operator~: not Ok()");
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

  auto err() && -> Option<E> {
    if (_inn.is_ok()) {
      return {};
    }
    return static_cast<E&&>(~_inn);
  }

  auto unwrap() const -> void {
    panicking::expect(_inn.is_ok(), "Result::unwrap: not Ok()");
  }

  auto unwrap_err() && -> E {
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
