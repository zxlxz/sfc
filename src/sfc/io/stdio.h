#pragma once

#include "mod.h"

namespace sfc::io {

class Stdout {
 public:
  static auto lock() -> sync::LockGuard;

  static auto is_tty() -> bool;

  static void flush();

  static void write_str(Str s);

  static void write_fmt(Str fmts, const auto&... args) {
    auto self = Stdout{};
    fmt::write(self, fmts, args...);
  }
};

void print(Str pattern, const auto&... args) {
  Stdout::write_fmt(pattern, args...);
}

void println(Str pattern, const auto&... args) {
  Stdout::write_fmt(pattern, args...);
  Stdout::write_str("\n");
}

}  // namespace sfc::io
