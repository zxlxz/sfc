#include "sfc/core/panicking.h"

#include "sfc/backtrace/frame.h"
#include "sfc/io/stdio.h"

namespace sfc::panicking {

void panic_str(Str msg) {
  const auto frames = backtrace::capture();

  auto buf = string::String{};
  auto out = fmt::Fmter{buf};
  out.write_str(msg);
  frames.iter().for_each_idx([&](usize i, const auto& frame) {
    out.write_fmt("\n{2}: {}", i, frame.func());
  });
  out.write_str("\n");
  
  io::Stdout{}.write_str(buf.as_str());

  throw PanicErr{};
}

}  // namespace sfc::panicking
