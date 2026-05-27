#pragma once

#include "sfc/core/panic.h"

namespace sfc::test {

using panic::SourceLoc;

struct ExpectInfo {
  bool _val;
  SourceLoc _loc;

  ExpectInfo(bool val, SourceLoc loc = {}) : _val{val}, _loc{loc} {}
};

void expect(ExpectInfo cond, const fmt::Fmts& fmts, const auto&... args) {
  if (cond._val) return;
  panic::panic_fmt(fmt::Args{fmts, args...}, cond._loc);
}

void expect_true(const auto& x, SourceLoc loc = SourceLoc::current()) {
  if (bool(x)) return;
  panic::panic_fmt(fmt::Args{"sfc::expect_true(`{}`) failed", x}, loc);
}

void expect_false(const auto& x, SourceLoc loc = SourceLoc::current()) {
  if (!bool(x)) return;
  panic::panic_fmt(fmt::Args{"sfc::expect_false(`{}`) failed", x}, loc);
}

void expect_eq(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  if (a == b) return;
  panic::panic_fmt(fmt::Args{"sfc::expect(`{}`==`{}`) failed", a, b}, loc);
}

void expect_ne(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  if (a != b) return;
  panic::panic_fmt(fmt::Args{"sfc::expect(`{}`!=`{}`) failed", a, b}, loc);
}

void expect_lt(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  if (a < b) return;
  panic::panic_fmt(fmt::Args{"sfc::expect(`{}`<`{}`) failed", a, b}, loc);
}

void expect_le(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  if (a <= b) return;
  panic::panic_fmt(fmt::Args{"sfc::expect(`{}`<=`{}`) failed", a, b}, loc);
}

void expect_gt(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  if (a > b) return;
  panic::panic_fmt(fmt::Args{"sfc::expect(`{}`>`{}`) failed", a, b}, loc);
}

void expect_ge(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  if (a >= b) return;
  panic::panic_fmt(fmt::Args{"sfc::expect(`{}`>=`{}`) failed", a, b}, loc);
}

inline void expect_flt_eq(f64 a, f64 b, u32 ulp = 4, SourceLoc loc = SourceLoc::current()) {
  if (num::flt_eq_ulp(a, b, ulp)) return;
  panic::panic_fmt(fmt::Args{"sfc::expect_flt(`{}`==`{}`) failed", a, b}, loc);
}

inline void expect_flt_ne(f64 a, f64 b, u32 ulp = 4, SourceLoc loc = SourceLoc::current()) {
  if (!num::flt_eq_ulp(a, b, ulp)) return;
  panic::panic_fmt(fmt::Args{"sfc::expect_flt(`{}`!=`{}`) failed", a, b}, loc);
}

}  // namespace sfc::test

namespace sfc {
using test::expect;
using test::expect_true;
using test::expect_false;

using test::expect_eq;
using test::expect_ne;
using test::expect_lt;
using test::expect_le;
using test::expect_gt;
using test::expect_ge;

using test::expect_flt_eq;
using test::expect_flt_ne;
}  // namespace sfc
