#include "sfc/test.h"

namespace sfc::fmt::test {

SFC_TEST(bool) {
  panicking::expect_eq(string::format("{}", true), "true");
  panicking::expect_eq(string::format("{}", false), "false");
}

SFC_TEST(str_align) {
  panicking::expect_eq(string::format("Hello {:<5}!", "x"), "Hello x    !");
  panicking::expect_eq(string::format("Hello {:-<5}!", "x"), "Hello x----!");
  panicking::expect_eq(string::format("Hello {:^5}!", "x"), "Hello   x  !");
  panicking::expect_eq(string::format("Hello {:>5}!", "x"), "Hello     x!");
}

}  // namespace sfc::fmt::test
