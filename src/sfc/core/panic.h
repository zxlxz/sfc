#pragma once

#include "sfc/core/fmt/args.h"
#include "sfc/core/num.h"
#include "sfc/core/trait.h"

namespace sfc::panic {

struct Location {
  const char* file;
  int line;

  Location(const char* file = __builtin_FILE(), int line = __builtin_LINE()) : file{file}, line{line} {}
};

[[noreturn]] void panic_imp(Location loc, const char* msg, usize len) noexcept;

[[noreturn]] void panic_fmt(Location loc, fmt::Fmts fmts, const auto&... args) noexcept;

}  // namespace sfc::panic
