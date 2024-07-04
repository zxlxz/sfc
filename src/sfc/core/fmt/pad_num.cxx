#include "sfc/test.h"

namespace sfc::fmt::test {

SFC_TEST(int) {
  panicking::assert_eq(string::format("{}", 0), "0");
  panicking::assert_eq(string::format("{}", +123), "123");
  panicking::assert_eq(string::format("{}", -123), "-123");

  panicking::assert_eq(string::format("{d}", 0), "0");
  panicking::assert_eq(string::format("{d}", +123), "123");
  panicking::assert_eq(string::format("{d}", -123), "-123");

  panicking::assert_eq(string::format("{x}", 0), "0");
  panicking::assert_eq(string::format("{x}", +123), "7b");
  panicking::assert_eq(string::format("{x}", -123), "-7b");

  panicking::assert_eq(string::format("{+X}", 0), "+0");
  panicking::assert_eq(string::format("{+X}", +123), "+7B");
  panicking::assert_eq(string::format("{+X}", -123), "-7B");

  panicking::assert_eq(string::format("{-b}", 0), " 0");
  panicking::assert_eq(string::format("{-b}", +123), " 1111011");
  panicking::assert_eq(string::format("{-b}", -123), "-1111011");

  panicking::assert_eq(string::format("{o}", 0), "0");
  panicking::assert_eq(string::format("{o}", +123), "173");
  panicking::assert_eq(string::format("{o}", -123), "-173");
}

SFC_TEST(int_sign) {
  panicking::assert_eq(string::format("{d}", 0), "0");
  panicking::assert_eq(string::format("{+d}", 0), "+0");
  panicking::assert_eq(string::format("{-d}", 0), " 0");

  panicking::assert_eq(string::format("{+}", 5), "+5");
  panicking::assert_eq(string::format("{-}", 5), " 5");

  panicking::assert_eq(string::format("{:05}!", +5), "00005!");
  panicking::assert_eq(string::format("{:05}!", -5), "-0005!");

  panicking::assert_eq(string::format("{:#010x}!", 27), "0x0000001b!");
}

SFC_TEST(int_align) {
  panicking::assert_eq(string::format("{>5d}", 42), "   42");
  panicking::assert_eq(string::format("{<5d}", 42), "42   ");
  panicking::assert_eq(string::format("{^5d}", 42), " 42  ");
  panicking::assert_eq(string::format("{=5d}", 42), "   42");

  panicking::assert_eq(string::format("{>5d}", -42), "  -42");
  panicking::assert_eq(string::format("{<5d}", -42), "-42  ");
  panicking::assert_eq(string::format("{^5d}", -42), " -42 ");
  panicking::assert_eq(string::format("{=5d}", -42), "-  42");

  panicking::assert_eq(string::format("{_>5d}", 42), "___42");
  panicking::assert_eq(string::format("{_<5d}", 42), "42___");
  panicking::assert_eq(string::format("{_^5d}", 42), "_42__");
  panicking::assert_eq(string::format("{_=5d}", 42), "___42");

  panicking::assert_eq(string::format("{_>5d}", -42), "__-42");
  panicking::assert_eq(string::format("{_<5d}", -42), "-42__");
  panicking::assert_eq(string::format("{_^5d}", -42), "_-42_");
  panicking::assert_eq(string::format("{_=5d}", -42), "-__42");

  panicking::assert_eq(string::format("{>05d}", 42), "00042");
  panicking::assert_eq(string::format("{<05d}", 42), "00042");
  panicking::assert_eq(string::format("{^05d}", 42), "00042");
  panicking::assert_eq(string::format("{=05d}", 42), "00042");

  panicking::assert_eq(string::format("{>05d}", -42), "-0042");
  panicking::assert_eq(string::format("{<05d}", -42), "-0042");
  panicking::assert_eq(string::format("{^05d}", -42), "-0042");
  panicking::assert_eq(string::format("{=05d}", -42), "-0042");
}

SFC_TEST(int_prefix) {
  panicking::assert_eq(string::format("{#x}", 16), "0x10");
  panicking::assert_eq(string::format("{#X}", 16), "0X10");
  panicking::assert_eq(string::format("{#b}", 16), "0b10000");
  panicking::assert_eq(string::format("{#o}", 16), "020");

  panicking::assert_eq(string::format("{#8x}", 16), "0x000010");
  panicking::assert_eq(string::format("{#8X}", 16), "0X000010");
  panicking::assert_eq(string::format("{#8b}", 16), "0b010000");
  panicking::assert_eq(string::format("{#8o}", 16), "00000020");
}

SFC_TEST(fix) {
  panicking::assert_eq(string::format("{.0f}", +123.456), "123");
  panicking::assert_eq(string::format("{.0f}", -123.456), "-123");

  panicking::assert_eq(string::format("{.2f}", +123.456), "123.46");
  panicking::assert_eq(string::format("{.2f}", -123.456), "-123.46");

  panicking::assert_eq(string::format("{+.2f}", +123.456), "+123.46");
  panicking::assert_eq(string::format("{-.2f}", -123.456), "-123.46");

  panicking::assert_eq(string::format("{+8.2f}", +123.456), " +123.46");
  panicking::assert_eq(string::format("{+8.2f}", -123.456), " -123.46");

  panicking::assert_eq(string::format("{-8.2f}", +123.456), "  123.46");
  panicking::assert_eq(string::format("{-8.2f}", -123.456), " -123.46");
}

SFC_TEST(exp) {
  panicking::assert_eq(string::format("{.2e}", +123.456), "1.23e+02");
  panicking::assert_eq(string::format("{.2e}", -123.456), "-1.23e+02");

  panicking::assert_eq(string::format("{+.2e}", +123.456), "+1.23e+02");
  panicking::assert_eq(string::format("{-.2e}", -123.456), "-1.23e+02");

  panicking::assert_eq(string::format("{+8.2e}", +123.456), "+1.23e+02");
  panicking::assert_eq(string::format("{+8.2e}", -123.456), "-1.23e+02");

  panicking::assert_eq(string::format("{-8.2e}", +123.456), " 1.23e+02");
  panicking::assert_eq(string::format("{-8.2e}", -123.456), "-1.23e+02");
}

}  // namespace sfc::fmt::test
