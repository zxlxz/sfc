#pragma once

#include "sfc/core/panic.h"

namespace sfc {

struct ExpectInfo {
  bool val;
  panic::Location loc;

  ExpectInfo(const auto& val, panic::Location loc = {}) : val{val}, loc{loc} {}
};

void expect(ExpectInfo info, const auto& fmts, const auto&... args) noexcept {
  if (info.val) {
    return;
  }
  panic::panic_fmt(info.loc, "sfc::expect(`{}`) failed", fmts, args...);
}

void expect_true(const auto& a, panic::Location loc = {}) noexcept {
  if (a) {
    return;
  }
  panic::panic_fmt(loc, "sfc::expect_true(`{}`) failed", a);
}

void expect_false(const auto& a, panic::Location loc = {}) noexcept {
  if (!a) {
    return;
  }
  panic::panic_fmt(loc, "sfc::expect_true(`{}`) failed", a);
}

void expect_eq(const auto& a, const auto& b, panic::Location loc = {}) noexcept {
  if (a == b) {
    return;
  }
  panic::panic_fmt(loc, "sfc::expect(`{}`==`{}`) failed", a, b);
}

void expect_ne(const auto& a, const auto& b, panic::Location loc = {}) noexcept {
  if (a != b) {
    return;
  }
  panic::panic_fmt(loc, "sfc::expect(`{}`!=`{}`) failed", a, b);
}

void expect_flt_eq(auto a, auto b, u32 ulp = 4, panic::Location loc = {}) noexcept {
  if (num::flt_eq_ulp(a, b, ulp)) {
    return;
  }
  panic::panic_fmt(loc, "sfc::expect.flt(`{}`==`{}`) failed", a, b);
}

void expect_flt_ne(auto a, auto b, u32 ulp = 4, panic::Location loc = {}) noexcept {
  if (!num::flt_eq_ulp(a, b, ulp)) {
    return;
  }
  panic::panic_fmt(loc, "sfc::expect.flt(`{}`!=`{}`) failed", a, b);
}

}  // namespace sfc
