#include "sfc/test/ut.h"

namespace sfc::num::test {

SFC_TEST(int_to_str) {
  char buf[32] = {};

  // dec
  panicking::expect_eq(to_str(buf, 0), "0");
  panicking::expect_eq(to_str(buf, +123), "123");
  panicking::expect_eq(to_str(buf, -123), "-123");

  // bin
  panicking::expect_eq(to_str(buf, 0, 'b'), "0");

  panicking::expect_eq(to_str(buf, +5, 'b'), "101");
  panicking::expect_eq(to_str(buf, +5, 'B'), "101");
  panicking::expect_eq(to_str(buf, -5, 'b'), "-101");
  panicking::expect_eq(to_str(buf, -5, 'B'), "-101");

  // oct
  panicking::expect_eq(to_str(buf, 0, 'o'), "0");
  panicking::expect_eq(to_str(buf, +8, 'o'), "10");
  panicking::expect_eq(to_str(buf, -8, 'o'), "-10");

  // hex
  panicking::expect_eq(to_str(buf, 0, 'x'), "0");
  panicking::expect_eq(to_str(buf, 255, 'x'), "ff");
  panicking::expect_eq(to_str(buf, 255, 'X'), "FF");
  panicking::expect_eq(to_str(buf, -255, 'x'), "-ff");
  panicking::expect_eq(to_str(buf, -255, 'X'), "-FF");
}

SFC_TEST(int_from_str) {
  // dec
  panicking::expect(from_str<int>("0"), Option{0});
  panicking::expect(from_str<int>("+123"), Option{123});
  panicking::expect(from_str<int>("-123"), Option{-123});

  // bin
  panicking::expect(from_str<int>("0", 2), Option{0b0});
  panicking::expect(from_str<int>("10", 2), Option{0b10});
  panicking::expect(from_str<int>("-10", 2), Option{-0b10});

  // oct
  panicking::expect(from_str<int>("0", 8), Option{0});
  panicking::expect(from_str<int>("10", 8), Option{010});
  panicking::expect(from_str<int>("-10", 8), Option{-010});

  // hex
  panicking::expect(from_str<int>("0", 16), Option{0x0});
  panicking::expect(from_str<int>("ff", 16), Option{0xff});
  panicking::expect(from_str<int>("FF", 16), Option{0xFF});
  panicking::expect(from_str<int>("-ff", 16), Option{-0xff});

  // neg
  panicking::expect_eq(from_str<i8>("-128"), Option<i8>{-128});
  panicking::expect_eq(from_str<i16>("-32768"), Option<i16>{-32768});

  // uint
  panicking::expect_eq(from_str<u8>("-1"), Option<u8>{});
  panicking::expect_eq(from_str<u32>("-1"), Option<u32>{});
  panicking::expect_eq(from_str<u32>("+1"), Option<u32>{1});

  // auto-radix
  panicking::expect_eq(from_str<int>("+0b101"), Option{+0b101});
  panicking::expect_eq(from_str<int>("-0b101"), Option{-0b101});
  panicking::expect_eq(from_str<int>("+010"), Option{+010});
  panicking::expect_eq(from_str<int>("-010"), Option{-010});
  panicking::expect_eq(from_str<int>("+0xFF"), Option{+0xFF});
  panicking::expect_eq(from_str<int>("-0xFF"), Option{-0xFF});

  // overflow
  panicking::expect_eq(from_str<u8>("256"), Option<u8>{});
  panicking::expect_eq(from_str<i8>("128"), Option<i8>{});
}

}  // namespace sfc::num::test
