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
  fmt::XArgs _args;
  SourceLoc _loc;
};

[[noreturn]] void panic_imp(PanicInfo info);

template <class... T>
[[noreturn]] void panic_fmt(const fmt::Args<T...>& args, SourceLoc loc = SourceLoc::current()) {
  panic::panic_imp({fmt::XArgs{args}, loc});
}

}  // namespace sfc::panic
