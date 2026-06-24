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
  const void* _args;
  SourceLoc _loc;
  void (*_write_buf)(fmt::SBuf& out, const void* args);
};

[[noreturn]] void panic_imp(PanicInfo info);

template <class... T>
[[noreturn]] void panic_fmt(const fmt::Args<T...>& args, SourceLoc loc = SourceLoc::current()) {
  auto write_buf = [](fmt::SBuf& out, const void* xargs) {
    fmt::write_fmt(out, *ptr::cast<const fmt::Args<T...>>(xargs));
  };
  const auto info = PanicInfo{&args, loc, write_buf};
  panic::panic_imp(info);
}

}  // namespace sfc::panic
