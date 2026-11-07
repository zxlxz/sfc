#include "sfc/core/panic.h"
#include "sfc/core/fmt.h"
#include "sfc/sys/io.h"
#include "sfc/sys/backtrace.h"

namespace sfc::panic {

void SourceLoc::fmt(fmt::Formatter& f) const {
  f.write_fmt("{}:{}", Str::from_cstr(file), line);
}

void panic_imp(PanicInfo info) {
  char raw_buf[4096];
  auto fmt_buf = fmt::Buf{raw_buf};

  auto fmter = fmt::Formatter{fmt_buf};
  fmter.write_fmt("thread panicked at {}\n{}\n", info._loc, info._args);

  auto bt = sys::Backtrace::capture();
  for (auto idx : Range{bt.len()}) {
    auto frame = bt.frame(idx);
    fmter.write_fmt(" {2}: {}\n", idx, frame.func);
  }

  auto sys_out = sys::StdIo::Error();
  (void)sys_out.write(fmt_buf.as_str().as_bytes());

  throw info;
}

}  // namespace sfc::panic
