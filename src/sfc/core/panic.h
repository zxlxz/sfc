#pragma once

#include "sfc/core/num.h"
#include "sfc/core/fmts.h"

namespace sfc::panic {

struct SourceLoc {
  const char* func;
  const char* file;
  int line;

 public:
  consteval static auto current(const char* func = __builtin_FUNCTION(),
                                const char* file = __builtin_FILE(),
                                int line = __builtin_LINE()) -> SourceLoc {
    return {func, file, line};
  }
};

struct PanicInfo {
  fmt::RawStr _msg;
  SourceLoc _loc;
};

[[noreturn]] void panic_imp(PanicInfo info);
[[noreturn]] void panic_fmt(const auto& args, SourceLoc loc = SourceLoc::current());
}  // namespace sfc::panic
