#pragma once

#include "sfc/core/num.h"
#include "sfc/core/fmts.h"

namespace sfc::panic {

struct SourceLoc {
  const char* func;
  const char* file;
  int line;

  consteval static auto current(const char* func = __builtin_FUNCTION(),
                                const char* file = __builtin_FILE(),
                                int line = __builtin_LINE()) -> SourceLoc {
    return {func, file, line};
  }
};

struct XFmt {
  fmt::Fmts _fmts;
  SourceLoc _loc;

 public:
  constexpr XFmt(fmt::Fmts fmts, SourceLoc loc) : _fmts{fmts}, _loc{loc} {}
  consteval XFmt(const char* fmts, SourceLoc loc = SourceLoc::current()) : _fmts{fmts}, _loc{loc} {}
};

struct Error {
  SourceLoc _loc;
};

[[noreturn]] void panic_imp(fmt::RawStr msg, SourceLoc loc);
[[noreturn]] void panic_fmt(const XFmt& fmts, const auto&... args);

void expect(bool x, const XFmt& fmts, const auto&... args) {
  if (x) return;
  panic::panic_fmt(fmts, args...);
}

void expect_true(const auto& x, SourceLoc loc = SourceLoc::current()) {
  if (x) return;
  panic::panic_fmt({fmt::Fmts{"sfc::expect_true(`{}`) failed"}, loc}, x);
}

void expect_false(const auto& x, SourceLoc loc = SourceLoc::current()) {
  if (!x) return;
  panic::panic_fmt({fmt::Fmts{"sfc::expect_false(`{}`) failed"}, loc}, x);
}

void expect_eq(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  if (a == b) return;
  panic::panic_fmt({fmt::Fmts{"sfc::expect(`{}`==`{}`) failed"}, loc}, a, b);
}

void expect_ne(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  if (a != b) return;
  panic::panic_fmt({fmt::Fmts{"sfc::expect(`{}`!=`{}`) failed"}, loc}, a, b);
}

void expect_lt(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  if (a < b) return;
  panic::panic_fmt({fmt::Fmts{"sfc::expect(`{}`<`{}`) failed"}, loc}, a, b);
}

void expect_le(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  if (a <= b) return;
  panic::panic_fmt({fmt::Fmts{"sfc::expect(`{}`<=`{}`) failed"}, loc}, a, b);
}

void expect_gt(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  if (a > b) return;
  panic::panic_fmt({fmt::Fmts{"sfc::expect(`{}`>`{}`) failed"}, loc}, a, b);
}

void expect_ge(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  if (a >= b) return;
  panic::panic_fmt({fmt::Fmts{"sfc::expect(`{}`>=`{}`) failed"}, loc}, a, b);
}

void expect_flt_eq(auto a, auto b, u32 ulp = 4, SourceLoc loc = SourceLoc::current()) {
  if (num::flt_eq_ulp(a, b, ulp)) return;
  panic::panic_fmt({fmt::Fmts{"sfc::expect_flt(`{}`==`{}`) failed"}, loc}, a, b);
}

void expect_flt_ne(auto a, auto b, u32 ulp = 4, SourceLoc loc = SourceLoc::current()) {
  if (!num::flt_eq_ulp(a, b, ulp)) return;
  panic::panic_fmt({fmt::Fmts{"sfc::expect_flt(`{}`!=`{}`) failed"}, loc}, a, b);
}

}  // namespace sfc::panic

namespace sfc {
using panic::expect;
using panic::expect_true;
using panic::expect_false;

using panic::expect_eq;
using panic::expect_ne;
using panic::expect_lt;
using panic::expect_le;
using panic::expect_gt;
using panic::expect_ge;

using panic::expect_flt_eq;
using panic::expect_flt_ne;
}  // namespace sfc
