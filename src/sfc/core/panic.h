#pragma once

#include "sfc/core/num.h"
#include "sfc/core/fmt/args.h"

namespace sfc::panic {

struct Location {
  const char* file;
  int line;

  static auto current(const char* file = __builtin_FILE(), int line = __builtin_LINE()) -> Location {
    return {file, line};
  }
};

struct Error {
  Location _loc;
};

[[noreturn]] void panic_imp(Location loc, const void* buf, usize buf_len);

template <class... T>
[[noreturn]] void panic_fmt(Location loc, fmt::Fmts<T...> fmts, const T&... args);

struct Condition {
  bool val;
  Location loc;
  [[gnu::always_inline]] Condition(const auto& val, Location loc = Location::current()) : val{val}, loc{loc} {}
};

template <class... T>
void expect(Condition cond, fmt::Fmts<T...> fmts, const T&... args) {
  if (cond.val) return;
  panic::panic_fmt(cond.loc, fmts, args...);
}

void expect_true(const auto& x, Location loc = Location::current()) {
  if (x) return;
  panic::panic_fmt(loc, "sfc::expect_true(`{}`) failed", x);
}

void expect_false(const auto& x, Location loc = Location::current()) {
  if (!x) return;
  panic::panic_fmt(loc, "sfc::expect_true(`{}`) failed", x);
}

void expect_eq(const auto& a, const auto& b, Location loc = Location::current()) {
  if (ops::eq(a, b)) return;
  panic::panic_fmt(loc, "sfc::expect(`{}`==`{}`) failed", a, b);
}

void expect_ne(const auto& a, const auto& b, Location loc = Location::current()) {
  if (ops::ne(a, b)) return;
  panic::panic_fmt(loc, "sfc::expect(`{}`!=`{}`) failed", a, b);
}

void expect_flt_eq(auto a, auto b, u32 ulp = 4, Location loc = Location::current()) {
  if (num::flt_eq_ulp(a, b, ulp)) return;
  panic::panic_fmt(loc, "sfc::expect_flt(`{}`==`{}`) failed", a, b);
}

void expect_flt_ne(auto a, auto b, u32 ulp = 4, Location loc = Location::current()) {
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
