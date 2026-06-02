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

template <class... T>
[[noreturn]] void panic_fmt(const fmt::Args<T...>& args, SourceLoc loc = SourceLoc::current()) {
  char buf[1024];
  auto out = fmt::SBuf{buf};
  fmt::write_fmt(out, args);
  panic::panic_imp({{buf, out._len}, loc});
}

}  // namespace sfc::panic
