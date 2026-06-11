#pragma once

#include "sfc/core/option.h"

namespace sfc::result {

template <class T, class E>
class [[nodiscard]] Result {
  static constexpr auto kTvCopyable = trait::tv_copy_<T> && trait::tv_copy_<E>;
  static constexpr auto kTvDroppable = trait::tv_drop_<T> && trait::tv_drop_<E>;

  u8 _tag;
  union {
    T _0;
    E _1;
  };

 public:
  constexpr Result(T t) noexcept : _tag{0}, _0{mem::move(t)} {}
  constexpr Result(E e) noexcept : _tag{1}, _1{mem::move(e)} {}

  ~Result() noexcept requires(kTvDroppable) = default;
  ~Result() noexcept {
    switch (_tag) {
      case 0:  mem::drop(_0); break;
      case 1:  mem::drop(_1); break;
      default: break;
    }
  }

  Result(const Result& other) noexcept requires(kTvCopyable) = default;
  Result(Result&& other) noexcept : _tag{0xFF} {
    switch (other._tag) {
      case 0:  ptr::write(&_0, mem::move(other._0)), _tag = 0; break;
      case 1:  ptr::write(&_1, mem::move(other._1)), _tag = 1; break;
      default: break;
    }
  }

  Result& operator=(const Result& other) noexcept requires(kTvCopyable) = default;
  Result& operator=(Result&& other) noexcept {
    if (this == &other) return *this;
    switch (_tag) {
      case 0:  mem::drop(_0), _tag = 0xFF; break;
      case 1:  mem::drop(_1), _tag = 0xFF; break;
      default: break;
    }
    switch (other._tag) {
      case 0:  ptr::write(&_0, mem::move(other._0)), _tag = 0; break;
      case 1:  ptr::write(&_1, mem::move(other._1)), _tag = 1; break;
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
    return mem::move(_0);
  }

  auto unwrap_err_unchecked() noexcept -> E {
    return mem::move(_1);
  }

  auto unwrap() && -> T {
    sfc::assert_fmt(_tag == 0, fmt::Args{"called `Result::unwrap()` on Err({})", _1});
    return mem::move(_0);
  }

  auto unwrap_err() && -> E {
    sfc::assert_fmt(_tag == 1, fmt::Args{"called `Result::unwrap_err()` on Ok({})", _0});
    return mem::move(_1);
  }

  auto unwrap_or(T default_val) && -> T {
    if (_tag == 0) return mem::move(_0);
    return mem::move(default_val);
  }

  auto expect(const auto& msg) && -> T {
    sfc::assert_fmt(_tag == 0, fmt::Args{"{}: Err({})", msg, _1});
    return mem::move(_0);
  }

  auto ok() && -> Option<T> {
    if (_tag != 0) return {};
    return mem::move(_0);
  }

  auto err() && -> Option<E> {
    if (_tag != 1) return {};
    return mem::move(_1);
  }

  template <class U>
  auto operator&(Result<U, E> res) && -> Result<U, E> {
    if (_tag == 0) return mem::move(res);
    return Result<U, E>{mem::move(_1)};
  }

  template <class F>
  auto operator|(Result<T, F> res) && -> Result<T, F> {
    if (_tag == 0) return Result<T, F>{mem::move(_0)};
    return mem::move(res);
  }

  template <class F, class ResultUE = FnOut<F, T>>
  auto and_then(F&& op) && -> ResultUE {
    if (_tag == 0) return op(mem::move(_0));
    return ResultUE{mem::move(_1)};
  }

  template <class O, class ResultTF = FnOut<O>>
  auto or_else(O&& op) && -> ResultTF {
    if (_tag == 0) return ResultTF{mem::move(_0)};
    return op();
  }

  template <class F, class U = FnOut<F, T>>
  auto map(F&& op) && -> Result<U, E> {
    if (_tag == 0) return Result<U, E>{op(mem::move(_0))};
    return Result<U, E>{mem::move(_1)};
  }

  template <class O, class F = FnOut<O, E>>
  auto map_err(O&& op) && -> Result<T, F> {
    if (_tag == 1) return Result<T, F>{op(mem::move(_1))};
    return Result<T, F>{mem::move(_0)};
  }

 public:
  // trait: ops::Eq
  auto eq(const Result& other) const -> bool {
    if (_tag == 0 && other._tag == 0) {
      return _0 == other._0;
    } else if (_tag == 1 && other._tag == 1) {
      return _1 == other._1;
    } else {
      return false;
    }
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    switch (_tag) {
      case 0:  f.write_fmt(fmt::Args{"Ok({})", _0}); break;
      case 1:  f.write_fmt(fmt::Args{"Err({})", _1}); break;
      default: f.write_str("Result(?)"); break;
    }
  }
};

template <class E>
class [[nodiscard]] Result<void, E> {
  static constexpr auto kTvCopyable = trait::tv_copy_<E>;
  static constexpr auto kTvDroppable = trait::tv_drop_<E>;

  u8 _tag;
  union {
    E _1;
  };

 public:
  constexpr Result() noexcept : _tag{0}, _1{} {}
  constexpr Result(E e) noexcept : _tag{1}, _1{mem::move(e)} {}

  ~Result() noexcept requires(kTvDroppable) = default;
  ~Result() requires(!kTvDroppable) {
    if (_tag == 1) {
      mem::drop(_1);
    }
  }

  Result(const Result& other) noexcept requires(kTvCopyable) = default;
  Result(Result&& other) noexcept : _tag{0} {
    if (other._tag == 1) {
      ptr::write(&_1, mem::move(other._1));
      _tag = 1;
    }
  }

  Result& operator=(const Result& other) noexcept requires(kTvCopyable) = default;
  Result& operator=(Result&& other) noexcept {
    if (this == &other) return *this;
    if (_tag == 1) {
      mem::drop(_1);
      _tag = 0;
    }
    if (other._tag == 1) {
      ptr::write(&_1, mem::move(other._1));
      _tag = 1;
    }
    return *this;
  }

  constexpr auto is_ok() const noexcept -> bool {
    return _tag == 0;
  }

  constexpr auto is_err() const noexcept -> bool {
    return _tag == 1;
  }

  void unwrap_unchecked() const noexcept {}

  auto unwrap_err_unchecked() -> E {
    return mem::move(_1);
  }

  void unwrap() && noexcept {
    sfc::assert_fmt(_tag == 0, fmt::Args{"Result::unwrap: not Ok()"});
  }

  auto unwrap_err() && -> E {
    sfc::assert_fmt(_tag == 1, fmt::Args{"Result::unwrap_err: not Err()"});
    return mem::move(_1);
  }

  auto err() && -> Option<E> {
    if (_tag != 1) return {};
    return mem::move(_1);
  }

 public:
  // trait: ops::Eq
  auto eq(const Result& other) const {
    if (_tag != other._tag) {
      return false;
    }
    if (_tag == 1 && other._tag == 1) {
      return _1 == other._1;
    }
    return true;
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

// trait: ops::Eq
template <class T, class E>
auto operator==(const Result<T, E>& left, const Result<T, E>& right) -> bool {
  return left.eq(right);
}

}  // namespace sfc::result

namespace sfc {
using result::Result;
}  // namespace sfc

#if !defined(__clang_analyzer__) && !defined(__INTELLISENSE__)
#if defined(__GNUC__) || defined(__clang__)
#define _TRY(expr)                          \
  ({                                        \
    auto _res = (expr);                     \
    if (_res.is_err()) {                    \
      return {_res.unwrap_err_unchecked()}; \
    }                                       \
    _res.unwrap_unchecked();                \
  })
#endif
#endif

#ifndef _TRY
#define _TRY(expr) expr.unwrap()
#endif
