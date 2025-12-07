#include "sfc/test/ut.h"

namespace sfc::fmt::test {

SFC_TEST(bool) {
  panicking::expect_eq(string::format("{}", true), "true");
  panicking::expect_eq(string::format("{}", false), "false");
}

SFC_TEST(str) {
  panicking::expect_eq(string::format("[{:5}]", "x"), "[x    ]");
  panicking::expect_eq(string::format("[{:<5}]", "x"), "[x    ]");
  panicking::expect_eq(string::format("[{:^5}]", "x"), "[  x  ]");
  panicking::expect_eq(string::format("[{:>5}]", "x"), "[    x]");
}

SFC_TEST(num_sign) {
  panicking::expect_eq(string::format("{d}", 0), "0");
  panicking::expect_eq(string::format("{+d}", 0), "+0");
  panicking::expect_eq(string::format("{-d}", 0), " 0");

  panicking::expect_eq(string::format("{+}", 5), "+5");
  panicking::expect_eq(string::format("{-}", 5), " 5");

  panicking::expect_eq(string::format("{+}", -5), "-5");
  panicking::expect_eq(string::format("{-}", -5), "-5");
}

SFC_TEST(num_fill) {
  panicking::expect_eq(string::format("{3}", +5), "  5");
  panicking::expect_eq(string::format("{3}", -5), " -5");

  panicking::expect_eq(string::format("{03}", +5), "005");
  panicking::expect_eq(string::format("{03}", -5), "-05");
}

SFC_TEST(num_alt) {
  panicking::expect_eq(string::format("{#x}", 5), "0x5");
  panicking::expect_eq(string::format("{#X}", 5), "0X5");
  panicking::expect_eq(string::format("{#b}", 5), "0b101");
  panicking::expect_eq(string::format("{#o}", 5), "05");

  panicking::expect_eq(string::format("{#4x}", 5), "0x05");
  panicking::expect_eq(string::format("{#4X}", 5), "0X05");
  panicking::expect_eq(string::format("{#4b}", 5), "0b101");
  panicking::expect_eq(string::format("{#4o}", 5), "0005");
}

SFC_TEST(num_align) {
  panicking::expect_eq(string::format("{>5d}", 42), "   42");
  panicking::expect_eq(string::format("{<5d}", 42), "42   ");
  panicking::expect_eq(string::format("{^5d}", 42), " 42  ");
  panicking::expect_eq(string::format("{=5d}", 42), "   42");

  panicking::expect_eq(string::format("{>5d}", -42), "  -42");
  panicking::expect_eq(string::format("{<5d}", -42), "-42  ");
  panicking::expect_eq(string::format("{^5d}", -42), " -42 ");
  panicking::expect_eq(string::format("{=5d}", -42), "-  42");
}

}  // namespace sfc::fmt::test
