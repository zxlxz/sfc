#pragma once

#include "sfc/core/num.h"
#include "sfc/core/fmts.h"

namespace sfc::panic {

struct SourceLoc {
  const char* file;
  int line;

  consteval static auto current(const char* file = __builtin_FILE(), int line = __builtin_LINE())
      -> SourceLoc {
    return {file, line};
  }
};

struct Error {
  SourceLoc _loc;
};

[[noreturn]] void panic_imp(fmt::RawStr msg, SourceLoc loc = SourceLoc::current());

template <class... T>
[[noreturn]] void panic_fmt(fmt::Args<T...> args, SourceLoc loc = SourceLoc::current());

template <class... T>
void expect(const auto& cond, fmt::Args<T...> args, SourceLoc loc = SourceLoc::current()) {
  if (cond) return;
  panic::panic_fmt(args, loc);
}

void expect_true(const auto& x, SourceLoc loc = SourceLoc::current()) {
  if (x) return;
  panic::panic_fmt(fmt::Args{"sfc::expect_true(`{}`) failed", x}, loc);
}

void expect_false(const auto& x, SourceLoc loc = SourceLoc::current()) {
  if (!x) return;
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

void expect_flt_eq(auto a, auto b, u32 ulp = 4, SourceLoc loc = SourceLoc::current()) {
  if (num::flt_eq_ulp(a, b, ulp)) return;
  panic::panic_fmt(fmt::Args{"sfc::expect_flt(`{}`==`{}`) failed", a, b}, loc);
}

void expect_flt_ne(auto a, auto b, u32 ulp = 4, SourceLoc loc = SourceLoc::current()) {
  if (!num::flt_eq_ulp(a, b, ulp)) return;
  panic::panic_fmt(fmt::Args{"sfc::expect_flt(`{}`!=`{}`) failed", a, b}, loc);
}

}  // namespace sfc::panic

namespace sfc {
using panic::expect;
using panic::expect_true;
using panic::expect_false;

using panic::expect_eq;
using panic::expect_ne;

using panic::expect_flt_eq;
using panic::expect_flt_ne;
}  // namespace sfc
