#pragma once

#include "sfc/core/mem.h"

namespace sfc::panicking {

struct Error {};

struct Location {
  cstr_t file = __builtin_FILE();
  i64 line    = __builtin_LINE();
};

struct RawStr {
  const char* _ptr;
  usize _len;

  template <usize N>
  RawStr(const char (&s)[N]) : _ptr{s}, _len{N - 1} {}

  RawStr(const auto& s) : _ptr{s._ptr}, _len{s._len} {}
};

struct LocationFmt {
  RawStr fmt;
  Location loc;

  LocationFmt(const auto& s, Location loc = {}) : fmt{s}, loc{loc} {}
};

[[noreturn]] void panic_fmt(LocationFmt fmt, const auto&... args);

void assert_fmt(const auto& cond, LocationFmt fmt, const auto&... args) {
  if (cond) {
    return;
  }

  panicking::panic_fmt(fmt, args...);
}

}  // namespace sfc::panicking

namespace sfc {
using panicking::assert_fmt;
using panicking::panic_fmt;
}  // namespace sfc
