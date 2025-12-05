#include "sfc/test.h"

namespace sfc::fmt::test {

SFC_TEST(pad_num_sign) {
  panicking::expect_eq(string::format("{d}", 0), "0");
  panicking::expect_eq(string::format("{+d}", 0), "+0");
  panicking::expect_eq(string::format("{-d}", 0), " 0");

  panicking::expect_eq(string::format("{+}", 5), "+5");
  panicking::expect_eq(string::format("{-}", 5), " 5");

  panicking::expect_eq(string::format("{+}", -5), "-5");
  panicking::expect_eq(string::format("{-}", -5), "-5");
}

SFC_TEST(pad_num_fill) {
  panicking::expect_eq(string::format("{:05}", +5), "00005");
  panicking::expect_eq(string::format("{:05}", -5), "-00005");
}

SFC_TEST(pad_num_prefix) {
  panicking::expect_eq(string::format("{#x}", 5), "0x5");
  panicking::expect_eq(string::format("{#X}", 5), "0X5");
  panicking::expect_eq(string::format("{#b}", 5), "0b101");
  panicking::expect_eq(string::format("{#o}", 5), "05");

  panicking::expect_eq(string::format("{#4x}", 5), "0x0005");
  panicking::expect_eq(string::format("{#4X}", 5), "0X0005");
  panicking::expect_eq(string::format("{#4b}", 5), "0b0101");
  panicking::expect_eq(string::format("{#4o}", 5), "00005");
}

SFC_TEST(pad_num_align) {
  panicking::expect_eq(string::format("{>5d}", 42), "   42");
  panicking::expect_eq(string::format("{<5d}", 42), "42   ");
  panicking::expect_eq(string::format("{^5d}", 42), " 42  ");
  panicking::expect_eq(string::format("{=5d}", 42), "   42");

  panicking::expect_eq(string::format("{>5d}", -42), "   -42");
  panicking::expect_eq(string::format("{<5d}", -42), "-42   ");
  panicking::expect_eq(string::format("{^5d}", -42), " -42  ");
  panicking::expect_eq(string::format("{=5d}", -42), "-   42");

  panicking::expect_eq(string::format("{_>5d}", 42), "___42");
  panicking::expect_eq(string::format("{_<5d}", 42), "42___");
  panicking::expect_eq(string::format("{_^5d}", 42), "_42__");
  panicking::expect_eq(string::format("{_=5d}", 42), "___42");

  panicking::expect_eq(string::format("{_>5d}", -42), "___-42");
  panicking::expect_eq(string::format("{_<5d}", -42), "-42___");
  panicking::expect_eq(string::format("{_^5d}", -42), "_-42__");
  panicking::expect_eq(string::format("{_=5d}", -42), "-___42");

  panicking::expect_eq(string::format("{>05d}", 42), "00042");
  panicking::expect_eq(string::format("{<05d}", 42), "00042");
  panicking::expect_eq(string::format("{^05d}", 42), "00042");
  panicking::expect_eq(string::format("{=05d}", 42), "00042");

  panicking::expect_eq(string::format("{>05d}", -42), "-00042");
  panicking::expect_eq(string::format("{<05d}", -42), "-00042");
  panicking::expect_eq(string::format("{^05d}", -42), "-00042");
  panicking::expect_eq(string::format("{=05d}", -42), "-00042");
}

}  // namespace sfc::fmt::test
