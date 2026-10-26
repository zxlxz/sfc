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
  panic::panic_fmt(cond._loc, fmts, args...);
}

void assert_eq(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  const auto cond = AssertCondition{a == b, loc};
  test::assert_(cond, "assert failed: `{}`\n  left: {},\n right: {}", "==", a, b);
}

void assert_ne(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  const auto cond = AssertCondition{a != b, loc};
  test::assert_(cond, "assert failed: `{}`\n  left: {},\n right: {}", "!=", a, b);
}

void assert_lt(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  const auto cond = AssertCondition{a < b, loc};
  test::assert_(cond, "assert failed: `{}`\n  left: {},\n right: {}", "<", a, b);
}

void assert_le(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  const auto cond = AssertCondition{a <= b, loc};
  test::assert_(cond, "assert failed: `{}`\n  left: {},\n right: {}", "<=", a, b);
}

void assert_gt(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  const auto cond = AssertCondition{a > b, loc};
  test::assert_(cond, "assert failed: `{}`\n  left: {},\n right: {}", ">", a, b);
}

void assert_ge(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  const auto cond = AssertCondition{a >= b, loc};
  test::assert_(cond, "assert failed: `{}`\n  left: {},\n right: {}", ">=", a, b);
}

inline void assert_flt_eq(f64 a, f64 b, f64 eps = 1e-12, SourceLoc loc = SourceLoc::current()) {
  const auto cond = AssertCondition{__builtin_fabs(a - b) < eps, loc};
  test::assert_(cond, "assert failed: `{}`\n  left: {},\n right: {}", "==", a, b);
}

inline void assert_flt_ne(f64 a, f64 b, f64 eps = 1e-12, SourceLoc loc = SourceLoc::current()) {
  const auto cond = AssertCondition{!(__builtin_fabs(a - b) < eps), loc};
  test::assert_(cond, "assert failed: `{}`\n  left: {},\n right: {}", "!=", a, b);
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
