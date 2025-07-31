#pragma once

#include "sfc/core/num.h"

namespace sfc::panicking {

struct PanicErr {};

struct Location {
  const char* file = __builtin_FILE();
  u32 line = __builtin_LINE();
};

struct PanicInfo {
  const char* val;
  Location loc;

  PanicInfo(const char* s) : val{s}, loc{} {}
};

struct AssertInfo {
  bool val;
  Location loc;

  AssertInfo(const auto& val, Location loc = {}) : val{val}, loc{loc} {}
};

[[noreturn]] void panic_imp(Location loc, const auto&... args);

[[noreturn]] void panic(PanicInfo info, const auto&... args);

void assert(AssertInfo info, const auto&... args) {
  info.val ? void(0) : panicking::panic_imp(info.loc, args...);
}

void assert_true(const auto& val, Location loc = {}) {
  if (val) {
    return;
  }
  panicking::panic_imp(loc, "assert(`{}`) failed", val);
}

void assert_false(const auto& val, Location loc = {}) {
  if (!val) {
    return;
  }
  panicking::panic_imp(loc, "assert(!`{}`) failed", val);
}

void assert_eq(const auto& a, const auto& b, Location loc = {}) {
  if (a == b) {
    return;
  }
  panicking::panic_imp(loc, "assert(`{}`==`{}`) failed", a, b);
}

void assert_ne(const auto& a, const auto& b, Location loc = {}) {
  if (a != b) {
    return;
  }
  panicking::panic_imp(loc, "assert(`{}`!=`{}`) failed", a, b);
}

inline void assert_flt_eq(auto a, auto b, u32 ulp = 4, Location loc = {}) {
  if (num::flt_eq_ulp(a, b, ulp)) {
    return;
  }
  panicking::panic_imp(loc, "assert.flt(`{}`==`{}`) failed", a, b);
}

inline void assert_flt_ne(auto a, auto b, u32 ulp = 4, Location loc = {}) {
  if (num::flt_eq_ulp(a, b, ulp)) {
    return;
  }
  panicking::panic_imp(loc, "assert.flt(`{}`!=`{}`) failed", a, b);
}

}  // namespace sfc::panicking
