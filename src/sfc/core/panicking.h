#pragma once

#include "sfc/core/num.h"

namespace sfc::panicking {

struct PanicErr {};

struct Location {
  const char* file = __builtin_FILE();
  int line = __builtin_LINE();
};

struct PanicInfo {
  const char* val;
  Location loc;

 public:
  PanicInfo(const char* s) noexcept : val{s}, loc{} {}
};

struct ExpectInfo {
  bool val;
  Location loc;

  ExpectInfo(const auto& val, Location loc = {}) : val{val}, loc{loc} {}
};

[[noreturn]] void panic_imp(Location loc, const auto&... args);

[[noreturn]] void panic(PanicInfo info, const auto&... args);

void expect(ExpectInfo info, const auto&... args) {
  if (info.val) {
    return;
  }
  panicking::panic_imp(info.loc, args...);
}

void expect_true(const auto& val, Location loc = {}) {
  if (val) {
    return;
  }
  panicking::panic_imp(loc, "expect(`{}`) failed", val);
}

void expect_false(const auto& val, Location loc = {}) {
  if (!val) {
    return;
  }
  panicking::panic_imp(loc, "expect(!`{}`) failed", val);
}

void expect_eq(const auto& a, const auto& b, Location loc = {}) {
  if (a == b) {
    return;
  }
  panicking::panic_imp(loc, "expect(`{}`==`{}`) failed", a, b);
}

void expect_ne(const auto& a, const auto& b, Location loc = {}) {
  if (a != b) {
    return;
  }
  panicking::panic_imp(loc, "expect(`{}`!=`{}`) failed", a, b);
}

inline void expect_flt_eq(auto a, auto b, u32 ulp = 4, Location loc = {}) {
  if (num::flt_eq_ulp(a, b, ulp)) {
    return;
  }
  panicking::panic_imp(loc, "expect.flt(`{}`==`{}`) failed", a, b);
}

inline void expect_flt_ne(auto a, auto b, u32 ulp = 4, Location loc = {}) {
  if (num::flt_eq_ulp(a, b, ulp)) {
    return;
  }
  panicking::panic_imp(loc, "expect.flt(`{}`!=`{}`) failed", a, b);
}

}  // namespace sfc::panicking
