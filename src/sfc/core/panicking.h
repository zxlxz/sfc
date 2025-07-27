#pragma once

#include "sfc/core/num.h"

namespace sfc::str {
struct Str;
}

namespace sfc::panicking {

struct PanicErr {};

struct Location {
  cstr_t file = __builtin_FILE();
  i64 line = __builtin_LINE();
};

template <class T>
struct XLocation {
  Location _loc;
  T _val;

 public:
  XLocation(auto&& val, Location loc = {}) : _loc{loc}, _val{val} {}
};

struct RawStr {
  const char* _ptr;
  usize _len;

 public:
  template <usize N>
  explicit RawStr(const char (&s)[N]) : _ptr{s}, _len{N - 1} {}

  explicit RawStr(const auto& s) : _ptr{s._ptr}, _len{s._len} {}

  operator str::Str() const;
};

[[noreturn]] void panic_imp(Location loc, const auto&... args);

[[noreturn]] void panic(XLocation<RawStr> fmts, const auto&... args) {
  panicking::panic_imp(fmts._loc, fmts._val, args...);
}

void assert(XLocation<bool> cond, const auto&... args) {
  cond._val ? void(0) : panicking::panic_imp(cond._loc, args...);
}

void assert_true(const auto& val, Location loc = {}) {
  !!val ? void(0) : panicking::panic_imp(loc, "assert(`{}`) failed", val);
}

void assert_false(const auto& val, Location loc = {}) {
  !val ? void(0) : panicking::panic_imp(loc, "assert(!`{}`) failed", val);
}

void assert_eq(const auto& a, const auto& b, Location loc = {}) {
  const auto cond = a == b;
  cond ? void(0) : panicking::panic_imp(loc, "assert(`{}`==`{}`) failed", a, b);
}

void assert_ne(const auto& a, const auto& b, Location loc = {}) {
  const auto cond = a != b;
  cond ? void(0) : panicking::panic_imp(loc, "assert(`{}`!=`{}`) failed", a, b);
}

inline void assert_flt_eq(auto a, auto b, u32 ulp = 4, Location loc = {}) {
  const auto cond = num::flt_eq_ulp(a, b, ulp);
  cond ? void(0) : panicking::panic_imp(loc, "assert.flt(`{}`==`{}`) failed", a, b);
}

inline void assert_flt_ne(auto a, auto b, u32 ulp = 4, Location loc = {}) {
  const auto cond = num::flt_eq_ulp(a, b, ulp);
  !cond ? void(0) : panicking::panic_imp(loc, "assert.flt(`{}`!=`{}`) failed", a, b);
}

}  // namespace sfc::panicking
