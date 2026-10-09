#include "sfc/io/stdio.h"
#include "sfc/test/test.h"

namespace sfc::io::test {

SFC_TEST(stdout) {
  auto& out = stdout();
  out.write_str("  stdout test\n");
  out.flush();
}

SFC_TEST(stderr) {
  auto& out = stderr();
  out.write_str("  stderr test\n");
  out.flush();
}
}  // namespace sfc::io::test
