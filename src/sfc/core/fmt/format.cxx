#include "sfc/test/test.h"

namespace sfc::fmt::test {

SFC_TEST(bool) {
  sfc::expect_eq(format("{}", true), "true");
  sfc::expect_eq(format("{}", false), "false");
}

SFC_TEST(str) {
  sfc::expect_eq(format("[{:5}]", "x"), "[x    ]");
  sfc::expect_eq(format("[{:<5}]", "x"), "[x    ]");
  sfc::expect_eq(format("[{:^5}]", "x"), "[  x  ]");
  sfc::expect_eq(format("[{:>5}]", "x"), "[    x]");
}

SFC_TEST(num_sign) {
  sfc::expect_eq(format("{d}", 0), "0");
  sfc::expect_eq(format("{+d}", 0), "+0");
  sfc::expect_eq(format("{-d}", 0), " 0");

  sfc::expect_eq(format("{+}", 5), "+5");
  sfc::expect_eq(format("{-}", 5), " 5");

  sfc::expect_eq(format("{+}", -5), "-5");
  sfc::expect_eq(format("{-}", -5), "-5");
}

SFC_TEST(num_fill) {
  sfc::expect_eq(format("{3}", +5), "  5");
  sfc::expect_eq(format("{3}", -5), " -5");

  sfc::expect_eq(format("{03}", +5), "005");
  sfc::expect_eq(format("{03}", -5), "-05");
}

SFC_TEST(num_alt) {
  sfc::expect_eq(format("{#x}", 5), "0x5");
  sfc::expect_eq(format("{#X}", 5), "0X5");
  sfc::expect_eq(format("{#b}", 5), "0b101");
  sfc::expect_eq(format("{#o}", 5), "05");

  sfc::expect_eq(format("{#4x}", 5), "0x05");
  sfc::expect_eq(format("{#4X}", 5), "0X05");
  sfc::expect_eq(format("{#4b}", 5), "0b101");
  sfc::expect_eq(format("{#4o}", 5), "0005");
}

SFC_TEST(num_align) {
  sfc::expect_eq(format("{>5d}", 42), "   42");
  sfc::expect_eq(format("{<5d}", 42), "42   ");
  sfc::expect_eq(format("{^5d}", 42), " 42  ");
  sfc::expect_eq(format("{=5d}", 42), "   42");

  sfc::expect_eq(format("{>5d}", -42), "  -42");
  sfc::expect_eq(format("{<5d}", -42), "-42  ");
  sfc::expect_eq(format("{^5d}", -42), " -42 ");
  sfc::expect_eq(format("{=5d}", -42), "-  42");
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
};

SFC_TEST(enum) {
  sfc::expect_eq(reflect::type_name<E1>(), "sfc::fmt::test::E1");
  sfc::expect_eq(format("{}", E1::A), "A");
  sfc::expect_eq(format("{}", E1::B), "B");
  sfc::expect_eq(format("{}", E1::C), "C");

  sfc::expect_eq(reflect::type_name<E2>(), "sfc::fmt::test::E2");
  sfc::expect_eq(format("{}", E2::X), "X");
  sfc::expect_eq(format("{}", E2::Y), "Y");
  sfc::expect_eq(format("{}", E2::Z), "Z");
}

}  // namespace sfc::fmt::test
