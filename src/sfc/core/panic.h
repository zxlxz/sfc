#pragma once

#include "sfc/core/num.h"
#include "sfc/core/fmt/args.h"

namespace sfc::panic {

struct SourceLoc {
  const char* file;
  int line;
  consteval static auto current(const char* file = __builtin_FILE(), int line = __builtin_LINE()) -> SourceLoc {
    return {file, line};
  }
};

struct Error {
  SourceLoc _loc;
};

template <class... T>
struct LocFmts {
  SourceLoc _loc;
  fmt::Fmts<T...> _fmts;
  consteval LocFmts(const auto& s, SourceLoc loc = SourceLoc::current()) : _loc{loc}, _fmts{s} {}
};

template <class... T>
using xfmts_t = trait::identity_t<LocFmts<T...>>;

[[noreturn]] void panic_imp(SourceLoc loc, const void* buf, usize buf_len);

template <class... T>
[[noreturn]] void panic_fmt(SourceLoc loc, fmt::fmts_t<T...> fmts, const T&... args);

template <class... T>
void expect(bool cond, xfmts_t<T...> xfmt, const T&... args) {
  if (cond) return;
  panic::panic_fmt(xfmt._loc, xfmt._fmts, args...);
}

void expect_true(const auto& x, SourceLoc loc = SourceLoc::current()) {
  if (x) return;
  panic::panic_fmt(loc, "sfc::expect_true(`{}`) failed", x);
}

void expect_false(const auto& x, SourceLoc loc = SourceLoc::current()) {
  if (!x) return;
  panic::panic_fmt(loc, "sfc::expect_true(`{}`) failed", x);
}

void expect_eq(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  if (ops::eq(a, b)) return;
  panic::panic_fmt(loc, "sfc::expect(`{}`==`{}`) failed", a, b);
}

void expect_ne(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  if (ops::ne(a, b)) return;
  panic::panic_fmt(loc, "sfc::expect(`{}`!=`{}`) failed", a, b);
}

void expect_flt_eq(auto a, auto b, u32 ulp = 4, SourceLoc loc = SourceLoc::current()) {
  if (num::flt_eq_ulp(a, b, ulp)) return;
  panic::panic_fmt(loc, "sfc::expect_flt(`{}`==`{}`) failed", a, b);
}

void expect_flt_ne(auto a, auto b, u32 ulp = 4, SourceLoc loc = SourceLoc::current()) {
  if (!num::flt_eq_ulp(a, b, ulp)) return;
  panic::panic_fmt(loc, "sfc::expect_flt(`{}`!=`{}`) failed", a, b);
}

}  // namespace sfc::panic

namespace sfc {
using panic::expect;
using panic::expect_false;
using panic::expect_true;

using panic::expect_eq;
using panic::expect_flt_eq;
using panic::expect_flt_ne;
using panic::expect_ne;
}  // namespace sfc
