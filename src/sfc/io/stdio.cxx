#include "sfc/io/stdio.h"
#include "sfc/test/ut.h"

namespace sfc::io::test {

SFC_TEST(stdout) {
  auto out = Stdout{};
  out.write_str("  stdout test\n");
  out.flush();
}

SFC_TEST(stderr) {
  auto out = Stderr{};
  out.write_str("  stderr test\n");
  out.flush();
}
}  // namespace sfc::io::test
