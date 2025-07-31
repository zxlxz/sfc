#include "sfc/core/panicking.h"

#include "sfc/backtrace/frame.h"
#include "sfc/io/stdio.h"

namespace sfc::panicking {

void panic_str(Str msg) {
  const auto frames = backtrace::capture();

  auto out = io::Stdout{}.lock();
  out.write_str(msg);
  frames.iter().for_each_idx([&](usize i, const auto& frame) {
    out.write_fmt("\n{2}: {}", i, frame);
  });
  out.write_str("\n");

  throw PanicErr{};
}

}  // namespace sfc::panicking
