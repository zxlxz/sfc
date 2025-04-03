#include "panicking.h"

#include "sfc/backtrace/frame.h"
#include "sfc/io/stdio.h"

namespace sfc::panicking {

void panic_str(Location loc, Str msg) {
  const auto frames = backtrace::capture();

  io::Stdout::write_str(msg);
  io::Stdout::write_fmt("  > {}:{}", loc.file, loc.line);

  frames.iter().for_each_idx([&](auto idx, const auto& frame) {
    io::Stdout::write_fmt("{2}: {}\n", idx, frame.func());
  });

  throw Error{};
}

}  // namespace sfc::panicking
