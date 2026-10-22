#pragma once

#include "sfc/core/option.h"

namespace sfc::result {

template <class T, class E>
class [[nodiscard]] Result;

template <class T = Unit>
struct Ok {
  T _0{};
};

template <class E>
struct Err {
  E _1{};
};

template <class T, class E>
class [[nodiscard]] Result {
  using TE = tuple::Tuple<T, E>;
  u8 _tag;
  union {
    T _0;
    E _1;
  };

 public:
  Result(T t) noexcept : _tag{0}, _0{mem::move(t)} {}
  Result(E e) noexcept : _tag{1}, _1{mem::move(e)} {}

  Result(Ok<T> ok) noexcept : _tag{0}, _0{mem::move(ok._0)} {}
  Result(Err<E> err) noexcept : _tag{1}, _1{mem::move(err._1)} {}

  ~Result() requires(trait::tv_drop_<TE>) = default;
  ~Result() {
    _tag == 0 ? mem::drop(_0) : mem::drop(_1);
  }

  Result(const Result& other) requires(trait::tv_copy_<TE>) = default;
  Result(Result&& other) requires(trait::tv_copy_<TE>) = default;
  Result(Result&& other) noexcept : _tag{other._tag} {
    _tag == 0 ? ptr::write(&_0, mem::move(other._0)) : ptr::write(&_1, mem::move(other._1));
  }

  Result& operator=(const Result& other) requires(trait::tv_copy_<TE>) = default;
  Result& operator=(Result&& other) requires(trait::tv_copy_<TE>) = default;
  Result& operator=(Result&& other) noexcept {
    if (this != &other) {
      _tag == 0 ? mem::drop(_0) : mem::drop(_1);
      _tag = other._tag;
      _tag == 0 ? ptr::write(&_0, mem::move(other._0)) : ptr::write(&_1, mem::move(other._1));
    }
    return *this;
  }

 public:
  explicit operator bool() const noexcept {
    return _tag == 0;
  }

  auto is_ok() const noexcept -> bool {
    return _tag == 0;
  }

  auto is_err() const noexcept -> bool {
    return _tag == 1;
  }

  auto as_ok() const noexcept -> Option<const T&> {
    if (_tag != 0) return {};
    return _0;
  }

  auto as_err() const noexcept -> Option<const E&> {
    if (_tag != 1) return {};
    return _1;
  }

  auto as_ref() const -> Result<const T&, E> {
    if (_tag == 0) return _0;
    return _1;
  }

  auto as_mut() -> Result<T&, E> {
    if (_tag == 0) return _0;
    return _1;
  }

  auto unwrap_unchecked() noexcept -> T {
    return mem::move(_0);
  }

  auto unwrap_err_unchecked() noexcept -> E {
    return _1;
  }

 public:
  auto unwrap() && -> T {
    sfc::assert_(this->is_ok(), "called `Result::unwrap()` on Err({})", _1);
    return mem::move(_0);
  }

  auto unwrap_err() && -> E {
    sfc::assert_(this->is_err(), "called `Result::unwrap_err()` on Ok({})", _0);
    return mem::move(_1);
  }

  auto unwrap_or(T default_val) && -> T {
    if (this->is_ok()) return mem::move(_0);
    return mem::move(default_val);
  }

  auto expect(const auto& msg) -> T {
    sfc::assert_(this->is_ok(), "{}: Err({})", msg, _1);
    return mem::move(_0);
  }

  auto ok() && -> Option<T> {
    if (this->is_ok()) return mem::move(_0);
    return {};
  }

  auto err() && -> Option<E> {
    if (this->is_err()) return mem::move(_1);
    return {};
  }

  template <class U>
  auto operator&(Result<U, E> res) && -> Result<U, E> {
    if (this->is_ok()) return mem::move(res);
    return mem::move(_1);
  }

  template <class F>
  auto operator|(Result<T, F> res) && -> Result<T, F> {
    if (this->is_err()) return mem::move(res);
    return mem::move(_0);
  }

  template <class F, class ResultUE = FnOut<F, T>>
  auto and_then(F&& op) && -> ResultUE {
    if (this->is_ok()) return op(mem::move(_0));
    return mem::move(_1);
  }

  template <class O, class ResultTF = FnOut<O>>
  auto or_else(O&& op) && -> ResultTF {
    if (this->is_err()) return op();
    return mem::move(_0);
  }

  template <class F, class U = FnOut<F, T>>
  auto map(F&& op) && -> Result<U, E> {
    if (this->is_ok()) return op(mem::move(_0));
    return mem::move(_1);
  }

  template <class O, class F = FnOut<O, E>>
  auto map_err(O&& op) && -> Result<T, F> {
    if (this->is_err()) return op(mem::move(_1));
    return mem::move(_0);
  }

 public:
  // trait: ops::Eq
  auto operator==(const Result& other) const -> bool {
    if (this == &other) return true;
    if (this->is_ok() && other.is_ok()) return _0 == other._0;
    if (this->is_err() && other.is_err()) return _1 == other._1;
    return false;
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    if (this->is_ok()) {
      f.write_fmt("Ok({})", _0);
    } else {
      f.write_fmt("Err({})", _1);
    }
  }
};

template <class T, class E>
class [[nodiscard]] Result<T&, E> {
  T* _0{};
  E _1{};

 public:
  Result(T& t) noexcept : _0{&t} {}
  Result(E e) noexcept : _1{e} {}

  Result(Ok<T&> ok) noexcept : _0{&ok._0} {}
  Result(Err<E> err) noexcept : _1{err._1} {}

 public:
  explicit operator bool() const noexcept {
    return _0 != nullptr;
  }

  auto is_ok() const noexcept -> bool {
    return _0 != nullptr;
  }

  auto is_err() const noexcept -> bool {
    return _0 == nullptr;
  }

  auto as_ok() const noexcept -> Option<T&> {
    if (_0 != nullptr) return {*_0};
    return {};
  }

  auto as_err() const noexcept -> Option<const E&> {
    if (_0 == nullptr) return {_1};
    return {};
  }

  auto unwrap_unchecked() noexcept -> T& {
    return *_0;
  }

  auto unwrap_err_unchecked() noexcept -> E {
    return _1;
  }

 public:
  auto unwrap() -> T& {
    sfc::assert_(this->is_ok(), "called `Result::unwrap()` on Err({})", this->_1);
    return *this->_0;
  }

  auto unwrap_err() -> E {
    sfc::assert_(this->is_err(), "called `Result::unwrap_err()` on Ok({})", this->_0);
    return this->_1;
  }

  auto unwrap_or(T default_val) -> T& {
    if (this->is_ok()) return *this->_0;
    return default_val;
  }

  auto expect(const auto& msg) -> T& {
    sfc::assert_(this->is_ok(), "{}: Err({})", msg, this->_1);
    return *this->_0;
  }

  auto ok() -> Option<T&> {
    if (this->is_err()) return {};
    return *_0;
  }

  auto err() -> Option<E> {
    if (this->is_ok()) return {};
    return _1;
  }

  template <class U>
  auto operator&(Result<U, E> res) -> Result<U, E> {
    if (this->is_ok()) return mem::move(res);
    return _1;
  }

  template <class F>
  auto operator|(Result<T&, F> res) -> Result<T&, F> {
    if (this->is_ok()) return *_0;
    return res;
  }

  template <class F, class ResultUE = FnOut<F, T>>
  auto and_then(F&& op) -> ResultUE {
    if (this->is_ok()) return op(*_0);
    return ResultUE{_1};
  }

  template <class O, class ResultTF = FnOut<O>>
  auto or_else(O&& op) -> ResultTF {
    if (this->is_ok()) return _0;
    return op();
  }

  template <class F, class U = FnOut<F, T>>
  auto map(F&& op) -> Result<U, E> {
    if (this->is_ok()) return op(*_0);
    return _1;
  }

  template <class O, class F = FnOut<O, E>>
  auto map_err(O&& op) -> Result<T, F> {
    if (this->is_err()) return op(_1);
    return *_0;
  }

 public:
  // trait: ops::Eq
  auto operator==(const Result& other) const -> bool {
    if (this == &other) return true;
    if (_0 && other._0) return _0 == other._0 || *_0 == *other._0;
    if (!_0 && !other._0) return _1 == other._1;
    return false;
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    if (this->is_ok()) {
      f.write_fmt("Ok({})", *_0);
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
#define _TRY(expr)                          \
  ({                                        \
    auto _res = (expr);                     \
    if (!_res) {                            \
      return {_res.unwrap_err_unchecked()}; \
    }                                       \
    _res.unwrap_unchecked();                \
  })
#else
#error "compiler not supported for _TRY macro"
#endif
#endif

#ifndef _TRY
#define _TRY(expr) expr.unwrap()
#endif
