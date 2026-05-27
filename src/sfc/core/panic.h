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

 public:
  static auto sbuf() -> fmt::SBuf {
    static thread_local char buf[1024];
    return fmt::SBuf{buf};
  }

  static auto from_args(const auto& args, SourceLoc loc = SourceLoc::current()) -> PanicInfo;
};

[[noreturn]] void panic_imp(PanicInfo info);

[[noreturn]] void panic_fmt(const auto& args, SourceLoc loc = SourceLoc::current()) {
  const auto info = PanicInfo::from_args(args, loc);
  panic::panic_imp(info);
}

}  // namespace sfc::panic
