#pragma once

#include "sfc/core/option.h"

namespace sfc::result {

template <class T, class E>
class [[nodiscard]] Result {
  T _0{};
  E _1{};

 public:
  constexpr Result(T t) noexcept : _0{mem::move(t)} {}
  constexpr Result(E e) noexcept : _1{e} {}

  constexpr auto is_ok() const noexcept -> bool {
    static constexpr auto kSuccess = E{};
    return _1 == kSuccess;
  }

  constexpr auto is_err() const noexcept -> bool {
    static constexpr auto kSuccess = E{};
    return _1 != kSuccess;
  }

  auto unwrap_unchecked() noexcept -> T {
    return mem::move(_0);
  }

  auto unwrap_err_unchecked() noexcept -> E {
    return mem::move(_1);
  }

 public:
  auto unwrap(this auto self) -> T {
    sfc::assert_(self.is_ok(), "called `Result::unwrap()` on Err({})", self._1);
    return mem::move(self._0);
  }

  auto unwrap_err(this auto self) -> E {
    sfc::assert_(self.is_err(), "called `Result::unwrap_err()` on Ok({})", self._0);
    return mem::move(self._1);
  }

  auto unwrap_or(this auto self, T default_val) -> T {
    if (self.is_ok()) return mem::move(self._0);
    return mem::move(default_val);
  }

  auto expect(this auto self, const auto& msg) -> T {
    sfc::assert_(self.is_ok(), "{}: Err({})", msg, self._1);
    return mem::move(self._0);
  }

  auto ok(this auto self) -> Option<T> {
    if (self.is_err()) return {};
    return mem::move(self._0);
  }

  auto err(this auto self) -> Option<E> {
    if (self.is_ok()) return {};
    return mem::move(self._1);
  }

  template <class U>
  auto operator&(this auto self, Result<U, E> res) -> Result<U, E> {
    if (self.is_ok()) return mem::move(res);
    return Result<U, E>{mem::move(self._1)};
  }

  template <class F>
  auto operator|(this auto self, Result<T, F> res) -> Result<T, F> {
    if (self.is_ok()) return Result<T, F>{mem::move(self._0)};
    return mem::move(res);
  }

  template <class F, class ResultUE = FnOut<F, T>>
  auto and_then(this auto self, F&& op) -> ResultUE {
    if (self.is_ok()) return op(mem::move(self._0));
    return ResultUE{mem::move(self._1)};
  }

  template <class O, class ResultTF = FnOut<O>>
  auto or_else(this auto self, O&& op) -> ResultTF {
    if (self.is_ok()) return ResultTF{mem::move(self._0)};
    return op();
  }

  template <class F, class U = FnOut<F, T>>
  auto map(this auto self, F&& op) -> Result<U, E> {
    if (self.is_ok()) return Result<U, E>{op(mem::move(self._0))};
    return Result<U, E>{mem::move(self._1)};
  }

  template <class O, class F = FnOut<O, E>>
  auto map_err(this auto self, O&& op) -> Result<T, F> {
    if (self.is_err()) return Result<T, F>{op(mem::move(self._1))};
    return Result<T, F>{mem::move(self._0)};
  }

 public:
  // trait: ops::Eq
  auto operator==(const Result& other) const -> bool {
    if (this->is_ok()) {
      return other.is_ok() && _0 == other._0;
    } else {
      return other.is_err() && _1 == other._1;
    }
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

template <class E>
class [[nodiscard]] Result<void, E> {
  struct T {};
  T _0{};
  E _1{};

 public:
  constexpr Result() noexcept : _0{} {}
  constexpr Result(E e) noexcept : _1{e} {}

  constexpr auto is_ok() const noexcept -> bool {
    static constexpr auto kSuccess = E{};
    return _1 == kSuccess;
  }

  constexpr auto is_err() const noexcept -> bool {
    static constexpr auto kSuccess = E{};
    return _1 != kSuccess;
  }

  void unwrap_unchecked() noexcept {}

  auto unwrap_err_unchecked() -> E {
    return _1;
  }

 public:
  void unwrap(this auto self) noexcept {
    sfc::assert_(self.is_ok(), "Result::unwrap: not Ok()");
  }

  auto unwrap_err(this auto self) -> E {
    sfc::assert_(self.is_err(), "Result::unwrap_err: not Err()");
    return mem::move(self._1);
  }

  auto ok([[maybe_unused]] this auto self) -> Option<void> {
    return {};
  }

  auto err(this auto self) -> Option<E> {
    if (self.is_ok()) return {};
    return mem::move(self._1);
  }

 public:
  // trait: ops::Eq
  auto operator==(const Result& other) const {
    if (this->is_err()) {
      return other.is_err() && _1 == other._1;
    }
    return true;
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    if (this->is_ok()) {
      f.write_str("Ok()");
    } else {
      f.write_fmt("Err({})", _1);
    }
  }
};

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
