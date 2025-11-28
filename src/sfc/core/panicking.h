#pragma once

#include "sfc/core/num.h"

namespace sfc::panicking {

struct Location {
  const char* file = __builtin_FILE();
  int line = __builtin_LINE();
};

struct PanicInfo {
  Location loc;
  const char* val;

 public:
  PanicInfo(const char* s, Location loc = {}) noexcept : loc{loc}, val{s} {}
};

struct ExpectInfo {
  Location loc;
  bool val;

  ExpectInfo(const auto& val, Location loc = {}) noexcept : loc{loc}, val{val} {}
};

[[noreturn]] void panic_fmt(Location loc, const auto&... args);

[[noreturn]] void panic(PanicInfo info, const auto&... args) {
  panicking::panic_fmt(info.loc, info.val, args...);
}

void expect(ExpectInfo info, const auto&... args) {
  if (info.val) {
    return;
  }
  panicking::panic_fmt(info.loc, args...);
}

void expect_true(const auto& val, Location loc = {}) {
  if (val) {
    return;
  }
  panicking::panic_fmt(loc, "expect(`{}`) failed", val);
}

void expect_false(const auto& val, Location loc = {}) {
  if (!val) {
    return;
  }
  panicking::panic_fmt(loc, "expect(!`{}`) failed", val);
}

void expect_eq(const auto& a, const auto& b, Location loc = {}) {
  if (a == b) {
    return;
  }
  panicking::panic_fmt(loc, "expect(`{}`==`{}`) failed", a, b);
}

void expect_ne(const auto& a, const auto& b, Location loc = {}) {
  if (a != b) {
    return;
  }
  panicking::panic_fmt(loc, "expect(`{}`!=`{}`) failed", a, b);
}

inline void expect_flt_eq(auto a, auto b, u32 ulp = 4, Location loc = {}) {
  if (num::flt_eq_ulp(a, b, ulp)) {
    return;
  }
  panicking::panic_fmt(loc, "expect.flt(`{}`==`{}`) failed", a, b);
}

inline void expect_flt_ne(auto a, auto b, u32 ulp = 4, Location loc = {}) {
  if (num::flt_eq_ulp(a, b, ulp)) {
    return;
  }
  panicking::panic_fmt(loc, "expect.flt(`{}`!=`{}`) failed", a, b);
}

}  // namespace sfc::panicking
