#pragma once

#include "sfc/core/panic.h"

namespace sfc::test {

using fmt::RawStr;
using panic::SourceLoc;

template<class ...T>
void assert_fmt(bool cond, const fmt::Args<T...>& args, SourceLoc loc = SourceLoc::current()) {
  if (cond) return;
  panic::panic_fmt(args, loc);
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
  test::assert_failed(loc, RawStr{"=="}, a, b);
}

void assert_ne(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  if (a != b) return;
  test::assert_failed(loc, RawStr{"!="}, a, b);
}

void assert_lt(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  if (a < b) return;
  test::assert_failed(loc, RawStr{"<"}, a, b);
}

void assert_le(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  if (a <= b) return;
  test::assert_failed(loc, RawStr{"<="}, a, b);
}

void assert_gt(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  if (a > b) return;
  test::assert_failed(loc, RawStr{">"}, a, b);
}

void assert_ge(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  if (a >= b) return;
  test::assert_failed(loc, RawStr{">="}, a, b);
}

inline void assert_flt_eq(f64 a, f64 b, u32 ulp = 4, SourceLoc loc = SourceLoc::current()) {
  if (num::flt_eq_ulp(a, b, ulp)) return;
  test::assert_failed(loc, RawStr{"=="}, a, b);
}

inline void assert_flt_ne(f64 a, f64 b, u32 ulp = 4, SourceLoc loc = SourceLoc::current()) {
  if (!num::flt_eq_ulp(a, b, ulp)) return;
  test::assert_failed(loc, RawStr{"!="}, a, b);
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
