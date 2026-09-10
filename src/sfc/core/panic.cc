#include "sfc/core/panic.h"
#include "sfc/core/fmt.h"
#include "sfc/sys/io.h"
#include "sfc/sys/backtrace.h"

namespace sfc::panic {

void panic_imp(PanicInfo info) {
  auto buf = fmt::Buf<4096>{};
  auto fmter = fmt::Formatter{buf};
  fmter.write_fmt("thread panicked at {}:{}\n", Str::from_cstr(info._loc.file), info._loc.line);
  fmter.write_val(info._args);
  fmter.write_str("\n");

  auto bt = sys::Backtrace::capture();
  for (auto idx : ops::Range{bt.len()}) {
    auto frame = bt.frame(idx);
    fmter.write_fmt(" {2}: {}\n", idx, frame.func);
  }

  auto out = sys::Stderr{};
  (void)out.write(buf.as_str().as_bytes());

  throw info;
}

[[noreturn]] void panic_fmt(fmt::Args args, SourceLoc loc) {
  const auto info = PanicInfo{args, loc};
  panic::panic_imp(info);
}

}  // namespace sfc::panic
