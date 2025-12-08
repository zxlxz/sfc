#pragma once

#include "sfc/core/trait.h"

namespace sfc::panicking {

struct Location {
  const char* file = __builtin_FILE();
  int line = __builtin_LINE();
};

struct ExpectInfo {
  bool val;
  Location loc;

  ExpectInfo(const auto& val, Location loc = {}) : val{val}, loc{loc} {}
};

[[noreturn]] void panic_fmt(Location loc, const auto&... args) noexcept;

void expect(ExpectInfo info, const auto& fmts, const auto&... args) noexcept {
  if (info.val) {
    return;
  }
  panicking::panic_fmt(info.loc, "panicking::expect(`{}`) failed", fmts, args...);
}

void expect(const auto& a, Location loc = {}) noexcept {
  if (a) {
    return;
  }
  panicking::panic_fmt(loc, "panicking::expect(`{}`) failed", a);
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

}  // namespace sfc::panicking
