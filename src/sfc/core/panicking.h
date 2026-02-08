#pragma once

#include "sfc/core/fmt/args.h"
#include "sfc/core/num.h"
#include "sfc/core/trait.h"

namespace sfc::panicking {

struct Location {
  const char* file;
  int line;

  Location(const char* file = __builtin_FILE(), int line = __builtin_LINE()) : file{file}, line{line} {}
};

struct ExpectInfo {
  bool val;
  Location loc;

  ExpectInfo(const auto& val, Location loc = {}) : val{val}, loc{loc} {}
};

[[noreturn]] void panic_str(Location loc, str::Str msg) noexcept;

template <class... T>
[[noreturn]] void panic_fmt(Location loc, fmt::Fmts fmts, const T&... args) noexcept;

void expect(ExpectInfo info, const auto& fmts, const auto&... args) noexcept {
  if (info.val) {
    return;
  }
  panicking::panic_fmt(info.loc, "panicking::expect(`{}`) failed", fmts, args...);
}

void expect_true(const auto& a, Location loc = {}) noexcept {
  if (a) {
    return;
  }
  panicking::panic_fmt(loc, "panicking::expect_true(`{}`) failed", a);
}

void expect_false(const auto& a, Location loc = {}) noexcept {
  if (!a) {
    return;
  }
  panicking::panic_fmt(loc, "panicking::expect_true(`{}`) failed", a);
}

void expect_eq(const auto& a, const auto& b, Location loc = {}) noexcept {
  if (a == b) {
    return;
  }
  panicking::panic_fmt(loc, "panicking::expect(`{}`==`{}`) failed", a, b);
}

void expect_ne(const auto& a, const auto& b, Location loc = {}) noexcept {
  if (a != b) {
    return;
  }
  panicking::panic_fmt(loc, "panicking::expect(`{}`!=`{}`) failed", a, b);
}

void expect_flt_eq(auto a, auto b, u32 ulp = 4, Location loc = {}) noexcept {
  if (num::flt_eq_ulp(a, b, ulp)) {
    return;
  }
  panicking::panic_fmt(loc, "panicking::expect.flt(`{}`==`{}`) failed", a, b);
}

void expect_flt_ne(auto a, auto b, u32 ulp = 4, Location loc = {}) noexcept {
  if (!num::flt_eq_ulp(a, b, ulp)) {
    return;
  }
  panicking::panic_fmt(loc, "panicking::expect.flt(`{}`!=`{}`) failed", a, b);
}

}  // namespace sfc::panicking
