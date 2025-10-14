#include "sfc/test.h"

namespace sfc::fmt::test {

SFC_TEST(bool) {
  panicking::expect_eq(string::format("{}", true), "true");
  panicking::expect_eq(string::format("{}", false), "false");
}

SFC_TEST(str_align) {
  panicking::expect_eq(string::format("[{:5}]", "x"), "[x    ]");
  panicking::expect_eq(string::format("[{:<5}]", "x"), "[x    ]");
  panicking::expect_eq(string::format("[{:^5}]", "x"), "[  x  ]");
  panicking::expect_eq(string::format("[{:>5}]", "x"), "[    x]");
}

}  // namespace sfc::fmt::test
