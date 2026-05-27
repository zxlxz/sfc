#pragma once

#include "sfc/core/panic.h"

namespace sfc::test {

using panic::SourceLoc;

struct AssertCond {
  bool _val;
  SourceLoc _loc;

  AssertCond(bool val, SourceLoc loc = {}) : _val{val}, _loc{loc} {}
};

template <class A, class B>
struct AssertExpr {
  const char* _op;
  const A& _left;
  const B& _right;

 public:
  void fmt(auto& f) const {
    f.write_fmt(fmt::Args{"`(left {} right)`\n  left = {}\n  right = {}", _op, _left, _right});
  }
};

[[noreturn]] void assert_failed(const auto& msg, SourceLoc loc = SourceLoc::current()) {
  char buf[1024];
  auto out = fmt::SBuf{buf};
  fmt::write(out, "assertion failed: {}", msg);
  panic::panic_imp(panic::PanicInfo{{out._ptr, out._len}, loc});
}

void assert_fmt(AssertCond cond, const fmt::Fmts& fmts, const auto&... args) {
  if (cond._val) return;
  test::assert_failed(fmt::Args{fmts, args...}, cond._loc);
}

void assert_eq(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  if (a == b) return;
  test::assert_failed(AssertExpr{"==", a, b}, loc);
}

void assert_ne(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  if (a != b) return;
  test::assert_failed(AssertExpr{"!=", a, b}, loc);
}

void assert_lt(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  if (a < b) return;
  test::assert_failed(AssertExpr{"<", a, b}, loc);
}

void assert_le(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  if (a <= b) return;
  test::assert_failed(AssertExpr{"<=", a, b}, loc);
}

void assert_gt(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  if (a > b) return;
  test::assert_failed(AssertExpr{">", a, b}, loc);
}

void assert_ge(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  if (a >= b) return;
  test::assert_failed(AssertExpr{">=", a, b}, loc);
}

inline void assert_flt_eq(f64 a, f64 b, u32 ulp = 4, SourceLoc loc = SourceLoc::current()) {
  if (num::flt_eq_ulp(a, b, ulp)) return;
  test::assert_failed(AssertExpr{"==", a, b}, loc);
}

inline void assert_flt_ne(f64 a, f64 b, u32 ulp = 4, SourceLoc loc = SourceLoc::current()) {
  if (!num::flt_eq_ulp(a, b, ulp)) return;
  test::assert_failed(AssertExpr{"!=", a, b}, loc);
}

}  // namespace sfc::test

namespace sfc {
using test::assert_fmt;

using test::assert_eq;
using test::assert_ne;
using test::assert_lt;
using test::assert_le;
using test::assert_gt;
using test::assert_ge;

using test::assert_flt_eq;
using test::assert_flt_ne;
}  // namespace sfc
