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
  static constexpr auto kSuccess = E{};

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

  ~Result() requires(trait::tv_drop_<T>) = default;
  Result(const Result& other) requires(trait::tv_copy_<T>) = default;
  Result& operator=(const Result& other) requires(trait::tv_copy_<T>) = default;

  ~Result() {
    _tag == 0 ? mem::drop(_0) : mem::drop(_1);
  }

  Result(Result&& other) noexcept : _tag{other._tag} {
    _tag == 0 ? ptr::write(&_0, mem::move(other._0)) : ptr::write(&_1, mem::move(other._1));
  }

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
    return _tag == 0 ? Option<const T&>{_0} : Option<const T&>{};
  }

  auto as_err() const noexcept -> Option<const E&> {
    return _tag == 0 ? Option<const E&>{_1} : Option<const E&>{};
  }

  auto as_ref() const -> Result<const T&, E> {
    return _tag == 0 ? Result<const T&, E>{_0} : Result<const T&, E>{_1};
  }

  auto as_mut() -> Result<T&, E> {
    return _tag == 0 ? Result<T&, E>{_0} : Result<T&, E>{_1};
  }

  auto unwrap_unchecked() noexcept -> T {
    return mem::move(_0);
  }

  auto unwrap_err_unchecked() noexcept -> E {
    return _1;
  }

 public:
  auto unwrap() && -> T {
    sfc::assert_(_tag == 0, "called `Result::unwrap()` on Err({})", _1);
    return mem::move(_0);
  }

  auto unwrap_err() && -> E {
    sfc::assert_(_tag == 1, "called `Result::unwrap_err()` on Ok({})", _0);
    return mem::move(_1);
  }

  auto unwrap_or(T default_val) && -> T {
    if (_tag == 0) return mem::move(_0);
    return mem::move(default_val);
  }

  auto expect(const auto& msg) -> T {
    sfc::assert_(_tag == 0, "{}: Err({})", msg, _1);
    return mem::move(_0);
  }

  auto ok() && -> Option<T> {
    return _tag == 0 ? Option<T>{mem::move(_0)} : Option<T>{};
  }

  auto err() && -> Option<E> {
    return _tag == 1 ? Option<E>{mem::move(_1)} : Option<E>{};
  }

  template <class U>
  auto operator&(Result<U, E> res) && -> Result<U, E> {
    return _tag == 0 ? Result<U, E>{mem::move(res._0)} : Result<U, E>{mem::move(_1)};
  }

  template <class F>
  auto operator|(Result<T, F> res) && -> Result<T, F> {
    return _tag == 0 ? Result<T, F>{mem::move(_0)} : mem::move(res);
  }

  template <class F, class ResultUE = FnOut<F, T>>
  auto and_then(F&& op) && -> ResultUE {
    return _tag == 0 ? op(mem::move(_0)) : ResultUE{mem::move(_1)};
  }

  template <class O, class ResultTF = FnOut<O>>
  auto or_else(O&& op) && -> ResultTF {
    return _tag == 0 ? ResultTF{mem::move(_0)} : op();
  }

  template <class F, class U = FnOut<F, T>>
  auto map(F&& op) && -> Result<U, E> {
    return _tag == 0 ? Result<U, E>{op(mem::move(_0))} : Result<U, E>{mem::move(_1)};
  }

  template <class O, class F = FnOut<O, E>>
  auto map_err(O&& op) && -> Result<T, F> {
    return _tag == 1 ? Result<T, F>{op(mem::move(_1))} : Result<T, F>{mem::move(_0)};
  }

 public:
  // trait: ops::Eq
  auto operator==(const Result& other) const -> bool {
    if (this == &other) return true;
    if (_tag == 0 && other._tag == 0) return _0 == other._0;
    if (_tag == 1 && other._tag == 1) return _1 == other._1;
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
    return Result<U, E>{_1};
  }

  template <class F>
  auto operator|(Result<T&, F> res) -> Result<T&, F> {
    if (this->is_ok()) return Result<T, F>{*_0};
    return res;
  }

  template <class F, class ResultUE = FnOut<F, T>>
  auto and_then(F&& op) -> ResultUE {
    if (this->is_ok()) return op(*_0);
    return ResultUE{_1};
  }

  template <class O, class ResultTF = FnOut<O>>
  auto or_else(O&& op) -> ResultTF {
    if (this->is_ok()) return ResultTF{*this->_0};
    return op();
  }

  template <class F, class U = FnOut<F, T>>
  auto map(F&& op) -> Result<U, E> {
    if (this->is_ok()) return Result<U, E>{op(*_0)};
    return Result<U, E>{_1};
  }

  template <class O, class F = FnOut<O, E>>
  auto map_err(O&& op) -> Result<T, F> {
    if (this->is_err()) return Result<T, F>{op(_1)};
    return Result<T, F>{*_0};
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
#endif
#endif

#ifndef _TRY
#define _TRY(expr) expr.unwrap()
#endif
