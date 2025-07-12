#include "sfc/core/panicking.h"

#include "sfc/backtrace/frame.h"
#include "sfc/io/stdio.h"

namespace sfc::panicking {

void panic_str(Location loc, Str msg) {
  const auto frames = backtrace::capture();

  auto out = io::Stdout::lock();

  out.write_str(msg);
  out.write_fmt("\n  > {}:{}\n", loc.file, loc.line);

  frames.iter().for_each_idx([&](usize i, const auto& frame) {
    out.write_fmt("{:2}: {}\n", i, frame.func());
  });

  __builtin_trap();
}

}  // namespace sfc::panicking
