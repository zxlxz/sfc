#pragma once

#include "sfc/core/panic.h"

namespace sfc::test {

using panic::SourceLoc;

struct AssertCondition {
  bool _val;
  SourceLoc _loc;

 public:
  constexpr AssertCondition(bool val, SourceLoc loc = SourceLoc::current()) : _val{val}, _loc{loc} {}
};

void assert_(AssertCondition cond, const fmt::Fmts& fmts, const auto&... args) {
  if (cond._val) return;
  panic::panic_fmt(fmt::Args{fmts, args...}, cond._loc);
}

[[noreturn]] void assert_failed(SourceLoc loc, const auto& expr, const auto&... args) {
  if constexpr (sizeof...(args) == 0) {
    const auto fmts = fmt::Fmts{"assert failed: `{}`"};
    panic::panic_fmt(fmt::Args{fmts, expr}, loc);
  } else if constexpr (sizeof...(args) == 1) {
    const auto fmts = fmt::Fmts{"assert failed: `{}`\n  value: {}"};
    panic::panic_fmt(fmt::Args{fmts, expr, args...}, loc);
  } else if constexpr (sizeof...(args) == 2) {
    const auto fmts = fmt::Fmts{"assert failed: `(left {} right)`\n  left: {},\n right: {}"};
    panic::panic_fmt(fmt::Args{fmts, expr, args...}, loc);
  }
}

void assert_eq(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  if (a == b) return;
  test::assert_failed(loc, fmt::CStr{"=="}, a, b);
}

void assert_ne(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  if (a != b) return;
  test::assert_failed(loc, fmt::CStr{"!="}, a, b);
}

void assert_lt(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  if (a < b) return;
  test::assert_failed(loc, fmt::CStr{"<"}, a, b);
}

void assert_le(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  if (a <= b) return;
  test::assert_failed(loc, fmt::CStr{"<="}, a, b);
}

void assert_gt(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  if (a > b) return;
  test::assert_failed(loc, fmt::CStr{">"}, a, b);
}

void assert_ge(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  if (a >= b) return;
  test::assert_failed(loc, fmt::CStr{">="}, a, b);
}

inline void assert_flt_eq(f64 a, f64 b, f64 eps = 1e-12, SourceLoc loc = SourceLoc::current()) {
  const auto diff = a - b;
  if (-eps < diff && diff < eps) return;
  test::assert_failed(loc, fmt::CStr{"=="}, a, b);
}

inline void assert_flt_ne(f64 a, f64 b, f64 eps = 1e-12, SourceLoc loc = SourceLoc::current()) {
  const auto diff = a - b;
  if (diff < -eps || eps < diff) return;
  test::assert_failed(loc, fmt::CStr{"!="}, a, b);
}

}  // namespace sfc::test

namespace sfc {
using test::assert_;

using test::assert_eq;
using test::assert_ne;
using test::assert_lt;
using test::assert_le;
using test::assert_gt;
using test::assert_ge;

using test::assert_flt_eq;
using test::assert_flt_ne;
}  // namespace sfc
