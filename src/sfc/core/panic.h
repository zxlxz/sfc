#pragma once

#include "sfc/core/fmts.h"

namespace sfc::panic {

struct SourceLoc {
  const char* func;
  const char* file;
  int line;

 public:
  static auto current(const char* func = __builtin_FUNCTION(),
                      const char* file = __builtin_FILE(),
                      int line = __builtin_LINE()) -> SourceLoc {
    return {func, file, line};
  }
};

struct PanicInfo {
  fmt::Args _args;
  SourceLoc _loc;
};

[[noreturn]] void panic_imp(PanicInfo info);

[[noreturn]] void panic_fmt(SourceLoc loc, fmt::Fmts fmts, const auto&... args) {
  const auto info = PanicInfo{{fmts, args...}, loc};
  panic::panic_imp(info);
}

}  // namespace sfc::panic
