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

struct PanicFmts : fmt::Fmts {
  SourceLoc _loc;

 public:
  constexpr PanicFmts(const auto& fmts, SourceLoc loc = SourceLoc::current()) : fmt::Fmts{fmts}, _loc{loc} {}
};

[[noreturn]] void panic_imp(PanicInfo info);

[[noreturn]] void panic_fmt(PanicFmts fmts, const auto&... args) {
  char buf[1024];
  auto out = fmt::SBuf{buf};
  fmt::write(out, fmts, args...);
  panic::panic_imp({{buf, out._len}, fmts._loc});
}

}  // namespace sfc::panic
