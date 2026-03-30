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

struct Bool {
  bool _val;
  SourceLoc _loc;
  Bool(bool val, SourceLoc loc = SourceLoc::current()) : _val{val}, _loc{loc} {}
};

struct Buffer {
  static constexpr auto BUF_LEN = 1024U;
  u8 _buf[BUF_LEN];
  usize _len;

  void write_str(auto s) {
    if (s._len == 0 || _len + s._len > BUF_LEN) return;
    __builtin_memcpy(_buf + _len, s._ptr, s._len);
    _len += s._len;
  }
};

[[noreturn]] void panic_imp(SourceLoc loc, const void* buf, usize buf_len);

template <class... T>
[[noreturn]] void panic_fmt(SourceLoc loc, fmt::fmts_t<T...> fmt, const T&... args) {
  if constexpr (sizeof...(args) == 0) {
    panic::panic_imp(loc, fmt._ptr, fmt._len);
  } else {
    auto buf = Buffer{};
    fmt::write(buf, fmt, args...);
    panic::panic_imp(loc, buf._buf, buf._len);
  }
}

template <class... T>
void expect(Bool val, fmt::fmts_t<T...> fmt, const T&... args) {
  if (val._val) return;
  panic::panic_fmt(val._loc, fmt, args...);
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
  if (a == b) return;
  panic::panic_fmt(loc, "sfc::expect(`{}`==`{}`) failed", a, b);
}

void expect_ne(const auto& a, const auto& b, SourceLoc loc = SourceLoc::current()) {
  if (a != b) return;
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
using panic::expect_true;
using panic::expect_false;

using panic::expect_eq;
using panic::expect_ne;

using panic::expect_flt_eq;
using panic::expect_flt_ne;
}  // namespace sfc
