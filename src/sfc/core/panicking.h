#pragma once

#include "sfc/core/num.h"

namespace sfc::panicking {

struct Location {
  const char* file = __builtin_FILE();
  int line = __builtin_LINE();
};

struct PanicInfo {
  const char* val;
  Location loc;
  PanicInfo(const char* s) noexcept : val{s}, loc{} {}
};

struct ExpectInfo {
  bool val;
  Location loc;
  ExpectInfo(const auto& val, Location loc = {}) : val{val}, loc{loc} {}
};

[[noreturn]] void panic_fmt(Location loc, const auto&... args) noexcept;

[[noreturn]] void panic(PanicInfo info, const auto&... args) noexcept {
  panic_fmt(info.loc, info.val, args...);
}

void expect(ExpectInfo info, const auto&... args) noexcept {
  if (info.val) {
    return;
  }
  panicking::panic_fmt(info.loc, args...);
}

void expect_true(const auto& val, Location loc = {}) noexcept {
  if (val) {
    return;
  }
  panicking::panic_fmt(loc, "panicking::expect_true(`{}`) failed", val);
}

void expect_false(const auto& val, Location loc = {}) noexcept {
  if (!val) {
    return;
  }
  panicking::panic_fmt(loc, "panicking::expect_false(`{}`) failed", val);
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
  if (num::flt_eq_ulp(a, b, ulp)) {
    return;
  }
  panicking::panic_fmt(loc, "panicking::expect.flt(`{}`!=`{}`) failed", a, b);
}

}  // namespace sfc::panicking
