#include "panicking.h"

#include "sfc/backtrace/frame.h"
#include "sfc/io/stdio.h"

namespace sfc::panicking {

void panic_str(Location loc, Str msg) {
  const auto backtrace = backtrace::Backtrace::capture();

  io::Stdout::write_str(msg);
  io::Stdout::write_fmt("\n  > {}:{}\n", loc.file, loc.line);

  for (auto i = 0U; i < backtrace.len(); ++i) {
    const auto frame = backtrace[i];
    io::Stdout::write_fmt("{2}: {}\n", i, frame.func());
  }

  throw Error{};
}

}  // namespace sfc::panicking
