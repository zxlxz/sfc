#pragma once

#include "mod.h"

namespace sfc::io {

class Stdout {
 public:
  static auto instance() -> Stdout&;

  auto is_tty() const -> bool;

  void flush();

  void write_str(Str s);

  void write_fmt(Str fmts, const auto&... args) {
    fmt::write(*this, fmts, args...);
  }
};

void print(Str pattern, const auto&... args) {
  auto& imp = Stdout::instance();
  imp.write_fmt(pattern, args...);
}

void println(Str pattern, const auto&... args) {
  auto& imp = Stdout::instance();
  imp.write_fmt(pattern, args...);
  imp.write_str("\n");
}

}  // namespace sfc::io
