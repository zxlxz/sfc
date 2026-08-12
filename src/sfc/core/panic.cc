#include "sfc/core/panic.h"
#include "sfc/core/fmt.h"
#include "sfc/sys/io.h"
#include "sfc/sys/backtrace.h"

namespace sfc::panic {

void writeln(const fmt::Args& args) {
  char buf[1024];
  auto out = Slice{buf};

  fmt::Formatter{out}.write_val(args);
  (void)out.write_str("\n");

  const auto str = Str{buf, sizeof(buf) - out.len()};
  (void)sys::Stderr().write(str.as_bytes());
}

void panic_imp(PanicInfo info) {
  writeln(fmt::Args{
      "thread panicked at {}:{}",
      Str::from_cstr(info._loc.file),
      info._loc.line,
  });
  writeln(info._args);

  auto bt = sys::Backtrace::capture();
  for (auto idx : ops::Range{bt.len()}) {
    auto frame = bt.frame(idx);
    writeln(fmt::Args{" {2}: {}", idx, frame.func});
  }
  throw info;
}

[[noreturn]] void panic_fmt(fmt::Args args, SourceLoc loc) {
  const auto info = PanicInfo{args, loc};
  panic::panic_imp(info);
}

}  // namespace sfc::panic
