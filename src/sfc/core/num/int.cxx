#include "sfc/test/ut.h"

namespace sfc::num::test {

SFC_TEST(fmt_dec) {
  panicking::expect_eq(string::format("{}", 0), "0");
  panicking::expect_eq(string::format("{}", +123), "123");
  panicking::expect_eq(string::format("{}", -123), "-123");

  panicking::expect_eq(string::format("{d}", 0), "0");
  panicking::expect_eq(string::format("{d}", +123), "123");
  panicking::expect_eq(string::format("{d}", -123), "-123");

  panicking::expect_eq(string::format("{b}", 0), "0");
  panicking::expect_eq(string::format("{b}", 123), "1111011");
  panicking::expect_eq(string::format("{b}", -123), "-1111011");
}

SFC_TEST(fmt_bin) {
  panicking::expect_eq(string::format("{b}", 0), "0");

  panicking::expect_eq(string::format("{b}", 5), "101");
  panicking::expect_eq(string::format("{b}", -5), "-101");
}

SFC_TEST(fmt_oct) {
  panicking::expect_eq(string::format("{o}", 0), "0");
  panicking::expect_eq(string::format("{o}", 8), "10");
  panicking::expect_eq(string::format("{o}", -8), "-10");
}

SFC_TEST(fmt_hex) {
  panicking::expect_eq(string::format("{x}", 0), "0");
  panicking::expect_eq(string::format("{x}", 255), "ff");
  panicking::expect_eq(string::format("{x}", -255), "-ff");

  panicking::expect_eq(string::format("{X}", 0), "0");
  panicking::expect_eq(string::format("{X}", 255), "FF");
  panicking::expect_eq(string::format("{X}", -255), "-FF");
}

SFC_TEST(parse_dec) {
  panicking::expect_eq(Str{"0"}.parse<i32>(), Option{0});
  panicking::expect_eq(Str{"123"}.parse<i32>(), Option{123});
  panicking::expect_eq(Str{"-123"}.parse<i32>(), Option{-123});
  panicking::expect_eq(Str{"+123"}.parse<i32>(), Option{123});

  panicking::expect(!Str{}.parse<i32>());
  panicking::expect(!Str{"abc"}.parse<i32>());
  panicking::expect(!Str{"123 "}.parse<i32>());
  panicking::expect(!Str{" 123 "}.parse<i32>());
  panicking::expect(!Str{"123.456"}.parse<i32>());

  panicking::expect(!Str{"++123"}.parse<i32>());
  panicking::expect(!Str{"-+123"}.parse<i32>());
  panicking::expect(!Str{"123+"}.parse<i32>());
  panicking::expect(!Str{"123-"}.parse<i32>());
}

SFC_TEST(parse_bin) {
  panicking::expect_eq(Str{"0b0"}.parse<i32>(), Option{0b0});
  panicking::expect_eq(Str{"-0b0"}.parse<i32>(), Option{-0b0});

  panicking::expect_eq(Str{"0b101"}.parse<i32>(), Option{0b101});
  panicking::expect_eq(Str{"+0b101"}.parse<i32>(), Option{+0b101});
  panicking::expect_eq(Str{"-0b101"}.parse<i32>(), Option{-0b101});
}

SFC_TEST(parse_oct) {
  panicking::expect_eq(Str{"0"}.parse<i32>(), Option{0});

  panicking::expect_eq(Str{"010"}.parse<i32>(), Option{010});
  panicking::expect_eq(Str{"+010"}.parse<i32>(), Option{+010});
  panicking::expect_eq(Str{"-010"}.parse<i32>(), Option{-010});
}

SFC_TEST(parse_hex) {
  panicking::expect_eq(Str{"0x0"}.parse<i32>(), Option{0x0});
  panicking::expect_eq(Str{"0xff"}.parse<i32>(), Option{0xff});
  panicking::expect_eq(Str{"0xFF"}.parse<i32>(), Option{0xFF});
}

}  // namespace sfc::num::test
