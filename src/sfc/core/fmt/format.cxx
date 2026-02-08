#include "sfc/test/test.h"

namespace sfc::fmt::test {

SFC_TEST(bool) {
  panicking::expect_eq(format("{}", true), "true");
  panicking::expect_eq(format("{}", false), "false");
}

SFC_TEST(str) {
  panicking::expect_eq(format("[{:5}]", "x"), "[x    ]");
  panicking::expect_eq(format("[{:<5}]", "x"), "[x    ]");
  panicking::expect_eq(format("[{:^5}]", "x"), "[  x  ]");
  panicking::expect_eq(format("[{:>5}]", "x"), "[    x]");
}

SFC_TEST(num_sign) {
  panicking::expect_eq(format("{d}", 0), "0");
  panicking::expect_eq(format("{+d}", 0), "+0");
  panicking::expect_eq(format("{-d}", 0), " 0");

  panicking::expect_eq(format("{+}", 5), "+5");
  panicking::expect_eq(format("{-}", 5), " 5");

  panicking::expect_eq(format("{+}", -5), "-5");
  panicking::expect_eq(format("{-}", -5), "-5");
}

SFC_TEST(num_fill) {
  panicking::expect_eq(format("{3}", +5), "  5");
  panicking::expect_eq(format("{3}", -5), " -5");

  panicking::expect_eq(format("{03}", +5), "005");
  panicking::expect_eq(format("{03}", -5), "-05");
}

SFC_TEST(num_alt) {
  panicking::expect_eq(format("{#x}", 5), "0x5");
  panicking::expect_eq(format("{#X}", 5), "0X5");
  panicking::expect_eq(format("{#b}", 5), "0b101");
  panicking::expect_eq(format("{#o}", 5), "05");

  panicking::expect_eq(format("{#4x}", 5), "0x05");
  panicking::expect_eq(format("{#4X}", 5), "0X05");
  panicking::expect_eq(format("{#4b}", 5), "0b101");
  panicking::expect_eq(format("{#4o}", 5), "0005");
}

SFC_TEST(num_align) {
  panicking::expect_eq(format("{>5d}", 42), "   42");
  panicking::expect_eq(format("{<5d}", 42), "42   ");
  panicking::expect_eq(format("{^5d}", 42), " 42  ");
  panicking::expect_eq(format("{=5d}", 42), "   42");

  panicking::expect_eq(format("{>5d}", -42), "  -42");
  panicking::expect_eq(format("{<5d}", -42), "-42  ");
  panicking::expect_eq(format("{^5d}", -42), " -42 ");
  panicking::expect_eq(format("{=5d}", -42), "-  42");
}

enum class E1 {
  A,
  B,
  C,
};

enum class E2 {
  X,
  Y,
  Z,
  _COUNT_,
};

SFC_TEST(enum) {
  panicking::expect_eq(str::type_name<E1>(), "sfc::fmt::test::E1");
  panicking::expect_eq(format("{}", E1::A), "0");
  panicking::expect_eq(format("{}", E1::B), "1");
  panicking::expect_eq(format("{}", E1::C), "2");

  panicking::expect_eq(str::type_name<E2>(), "sfc::fmt::test::E2");
  panicking::expect_eq(format("{}", E2::X), "X");
  panicking::expect_eq(format("{}", E2::Y), "Y");
  panicking::expect_eq(format("{}", E2::Z), "Z");
}

}  // namespace sfc::fmt::test
