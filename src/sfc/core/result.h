#pragma once

#include "sfc/core/option.h"

namespace sfc::result {

template <class T, class E>
class [[nodiscard]] Result;

template <class T = Unit>
struct Ok {
  T _ok{};
};

template <class E>
struct Err {
  E _err{};
};

template <class T>
struct Ref {
  T* _ptr;

 public:
  Ref(T* p) noexcept : _ptr{p} {}
  Ref(T& t) noexcept : _ptr{&t} {}

 public:
  operator T&() const noexcept {
    return *_ptr;
  }

  auto operator==(const T& other) const noexcept -> bool {
    return *_ptr == other;
  }

  void fmt(auto& f) const {
    f.write_fmt("{}", *_ptr);
  }
};

template <class T, class E>
struct RawResult {
  u8 _tag;
  union {
    T _ok;
    E _err;
  };

 public:
  RawResult(T t) noexcept : _tag{0}, _ok{mem::move(t)} {}
  RawResult(E e) noexcept : _tag{1}, _err{mem::move(e)} {}

  RawResult(Ok<T> ok) noexcept : _tag{0}, _ok{mem::move(ok._ok)} {}
  RawResult(Err<E> err) noexcept : _tag{1}, _err{mem::move(err._err)} {}

  ~RawResult() {
    _tag == 0 ? mem::drop(_ok) : mem::drop(_err);
  }

  RawResult(RawResult&& other) noexcept : _tag{other._tag} {
    _tag == 0 ? ptr::write(&_ok, mem::move(other._ok)) : ptr::write(&_err, mem::move(other._err));
  }

  RawResult& operator=(RawResult&& other) noexcept {
    if (this != &other) {
      _tag == 0 ? mem::drop(_ok) : mem::drop(_err);
      _tag = other._tag;
      _tag == 0 ? ptr::write(&_ok, mem::move(other._ok)) : ptr::write(&_err, mem::move(other._err));
    }
    return *this;
  }

 public:
  auto is_ok() const noexcept -> bool {
    return _tag == 0;
  }

  auto is_err() const noexcept -> bool {
    return _tag == 1;
  }
};

template <trait::tv_copy_ T, trait::tv_copy_ E>
struct RawResult<T, E> {
  u8 _tag;
  union {
    T _ok;
    E _err;
  };

 public:
  RawResult(T t) noexcept : _tag{0}, _ok{mem::move(t)} {}
  RawResult(E e) noexcept : _tag{1}, _err{mem::move(e)} {}

  RawResult(Ok<T> ok) noexcept : _tag{0}, _ok{mem::move(ok._ok)} {}
  RawResult(Err<E> err) noexcept : _tag{1}, _err{mem::move(err._err)} {}

 public:
  auto is_ok() const noexcept -> bool {
    return _tag == 0;
  }

  auto is_err() const noexcept -> bool {
    return _tag == 1;
  }
};

template <class T, class E>
struct RawResult<T&, E> {
  using R = Ref<T>;
  R _ok{nullptr};
  E _err{};

 public:
  RawResult(T& t) noexcept : _ok{t} {}
  RawResult(E e) noexcept : _err{e} {}

  RawResult(Ok<T&> ok) noexcept : _ok{ok._ok} {}
  RawResult(Err<E> err) noexcept : _err{err._err} {}

 public:
  auto is_ok() const noexcept -> bool {
    return _ok._ptr != nullptr;
  }

  auto is_err() const noexcept -> bool {
    return _ok._ptr == nullptr;
  }
};

template <class T, class E>
class [[nodiscard]] Result : RawResult<T, E> {
  using Inn = result::RawResult<T, E>;
  using Inn::_tag;
  using Inn::_ok;
  using Inn::_err;

 public:
  using Inn::Inn;
  using Inn::is_ok;
  using Inn::is_err;

 public:
  explicit operator bool() const noexcept {
    return this->is_ok();
  }

  auto as_ok() const noexcept -> Option<const T&> {
    if (this->is_ok()) return {_ok};
    return {};
  }

  auto as_err() const noexcept -> Option<const E&> {
    if (this->is_err()) return {_err};
    return {};
  }

  auto as_ref() const -> Result<const T&, E> {
    if (this->is_ok()) return {_ok};
    return {_err};
  }

  auto as_mut() -> Result<T&, E> {
    if (this->is_ok()) return {_ok};
    return {_err};
  }

  auto unwrap_unchecked() noexcept -> T {
    return mem::move(_ok);
  }

  auto unwrap_err_unchecked() noexcept -> E {
    return _err;
  }

 public:
  auto unwrap() && -> T {
    sfc::assert_(this->is_ok(), "called `Result::unwrap()` on Err({})", _err);
    return mem::move(_ok);
  }

  auto unwrap_err() && -> E {
    sfc::assert_(this->is_err(), "called `Result::unwrap_err()` on Ok({})", _ok);
    return mem::move(_err);
  }

  auto unwrap_or(T default_val) && -> T {
    if (this->is_ok()) return mem::move(_ok);
    return mem::move(default_val);
  }

  auto expect(const auto& msg) -> T {
    sfc::assert_(this->is_ok(), "{}: Err({})", msg, _err);
    return mem::move(_ok);
  }

  auto ok() && -> Option<T> {
    if (this->is_ok()) return mem::move(_ok);
    return {};
  }

  auto err() && -> Option<E> {
    if (this->is_err()) return mem::move(_err);
    return {};
  }

  template <class U>
  auto operator&(Result<U, E> res) && -> Result<U, E> {
    if (this->is_ok()) return mem::move(res);
    return mem::move(_err);
  }

  template <class F>
  auto operator|(Result<T, F> res) && -> Result<T, F> {
    if (this->is_err()) return mem::move(res);
    return mem::move(_ok);
  }

  template <class F, class ResultUE = FnOut<F, T>>
  auto and_then(F&& op) && -> ResultUE {
    if (this->is_ok()) return op(mem::move(_ok));
    return mem::move(_err);
  }

  template <class O, class ResultTF = FnOut<O>>
  auto or_else(O&& op) && -> ResultTF {
    if (this->is_err()) return op();
    return mem::move(_ok);
  }

  template <class F, class U = FnOut<F, T>>
  auto map(F&& op) && -> Result<U, E> {
    if (this->is_ok()) return op(mem::move(_ok));
    return mem::move(_err);
  }

  template <class O, class F = FnOut<O, E>>
  auto map_err(O&& op) && -> Result<T, F> {
    if (this->is_err()) return op(mem::move(_err));
    return mem::move(_ok);
  }

 public:
  // trait: ops::Eq
  auto operator==(const Result& other) const -> bool {
    if (this == &other) return true;
    if (this->is_ok() && other.is_ok()) return _ok == other._ok;
    if (this->is_err() && other.is_err()) return _err == other._err;
    return false;
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    if (this->is_ok()) {
      f.write_fmt("Ok({})", _ok);
    } else {
      f.write_fmt("Err({})", _err);
    }
  }
};

template <class T, class E>
class [[nodiscard]] Result<T&, E> : RawResult<T&, E> {
  using Inn = result::RawResult<T&, E>;
  using Inn::_ok;
  using Inn::_err;

 public:
  using Inn::Inn;
  using Inn::is_ok;
  using Inn::is_err;

 public:
  explicit operator bool() const noexcept {
    return this->is_ok();
  }

  auto as_ok() const noexcept -> Option<const T&> {
    if (this->is_ok()) return {_ok};
    return {};
  }

  auto as_err() const noexcept -> Option<const E&> {
    if (this->is_err()) return {_err};
    return {};
  }

  auto unwrap_unchecked() noexcept -> T& {
    return _ok;
  }

  auto unwrap_err_unchecked() noexcept -> E {
    return _err;
  }

 public:
  auto unwrap() -> T& {
    sfc::assert_(this->is_ok(), "called `Result::unwrap()` on Err({})", this->_err);
    return this->_ok;
  }

  auto unwrap_err() -> E {
    sfc::assert_(this->is_err(), "called `Result::unwrap_err()` on Ok({})", this->_ok);
    return this->_err;
  }

  auto unwrap_or(T default_val) -> T& {
    if (this->is_ok()) return this->_ok;
    return default_val;
  }

  auto expect(const auto& msg) -> T& {
    sfc::assert_(this->is_ok(), "{}: Err({})", msg, this->_err);
    return this->_ok;
  }

  auto ok() -> Option<T&> {
    if (this->is_err()) return {};
    return _ok;
  }

  auto err() -> Option<E> {
    if (this->is_ok()) return {};
    return _err;
  }

  template <class U>
  auto operator&(Result<U, E> res) -> Result<U, E> {
    if (this->is_ok()) return mem::move(res);
    return _err;
  }

  template <class F>
  auto operator|(Result<T&, F> res) -> Result<T&, F> {
    if (this->is_ok()) return _ok;
    return res;
  }

  template <class F, class ResultUE = FnOut<F, T>>
  auto and_then(F&& op) -> ResultUE {
    if (this->is_ok()) return op(_ok);
    return ResultUE{_err};
  }

  template <class O, class ResultTF = FnOut<O>>
  auto or_else(O&& op) -> ResultTF {
    if (this->is_ok()) return _ok;
    return op();
  }

  template <class F, class U = FnOut<F, T>>
  auto map(F&& op) -> Result<U, E> {
    if (this->is_ok()) return op(_ok);
    return _err;
  }

  template <class O, class F = FnOut<O, E>>
  auto map_err(O&& op) -> Result<T, F> {
    if (this->is_err()) return op(_err);
    return _ok;
  }

 public:
  // trait: ops::Eq
  auto operator==(const Result& other) const -> bool {
    if (this == &other) return true;
    if (_ok && other._ok) return _ok == other._ok;
    if (!_ok && !other._ok) return _err == other._err;
    return false;
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    if (this->is_ok()) {
      f.write_fmt("Ok({})", _ok);
    } else {
      f.write_fmt("Err({})", _err);
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
