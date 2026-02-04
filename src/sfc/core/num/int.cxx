#include "sfc/test/test.h"

namespace sfc::num::test {

SFC_TEST(int_to_str) {
  char buf[32] = {};

  // dec
  panicking::expect_eq(Int{0}.to_str(buf), "0");
  panicking::expect_eq(Int{+123}.to_str(buf), "123");
  panicking::expect_eq(Int{-123}.to_str(buf), "-123");
}

SFC_TEST(bin_to_str) {
  char buf[32] = {};

  // bin
  panicking::expect_eq(Int{0}.to_str(buf, 'b'), "0");

  panicking::expect_eq(Int{+5}.to_str(buf, 'b'), "101");
  panicking::expect_eq(Int{+5}.to_str(buf, 'B'), "101");
  panicking::expect_eq(Int{-5}.to_str(buf, 'b'), "-101");
  panicking::expect_eq(Int{-5}.to_str(buf, 'B'), "-101");

  // oct
  panicking::expect_eq(Int{0}.to_str(buf, 'o'), "0");
  panicking::expect_eq(Int{+8}.to_str(buf, 'o'), "10");
  panicking::expect_eq(Int{-8}.to_str(buf, 'o'), "-10");

  // hex
  panicking::expect_eq(Int{0}.to_str(buf, 'x'), "0");
  panicking::expect_eq(Int{255}.to_str(buf, 'x'), "ff");
  panicking::expect_eq(Int{255}.to_str(buf, 'X'), "FF");
  panicking::expect_eq(Int{-255}.to_str(buf, 'x'), "-ff");
  panicking::expect_eq(Int{-255}.to_str(buf, 'X'), "-FF");
}

SFC_TEST(int_from_str) {
  // dec
  panicking::expect(Int<int>::from_str("0"), Option{0});
  panicking::expect(Int<int>::from_str("+123"), Option{123});
  panicking::expect(Int<int>::from_str("-123"), Option{-123});

  // neg
  panicking::expect_eq(Int<i8>::from_str("-128"), Option<i8>{-128});
  panicking::expect_eq(Int<i16>::from_str("-32768"), Option<i16>{-32768});

  // uint
  panicking::expect_eq(Int<u8>::from_str("-1"), Option<u8>{});
  panicking::expect_eq(Int<u32>::from_str("-1"), Option<u32>{});
  panicking::expect_eq(Int<u32>::from_str("+1"), Option<u32>{1});

  // unsigned overflow
  panicking::expect_eq(Int<u8>::from_str("255"), Option<u8>{255});
  panicking::expect_eq(Int<u8>::from_str("256"), Option<u8>{});

  // signed overflow
  panicking::expect_eq(Int<i8>::from_str("-128"), Option<i8>{-128});
  panicking::expect_eq(Int<i8>::from_str("128"), Option<i8>{});
}

SFC_TEST(bin_from_str) {
  // dec
  panicking::expect(Int<int>::from_str("0"), Option{0});
  panicking::expect(Int<int>::from_str("+123"), Option{123});
  panicking::expect(Int<int>::from_str("-123"), Option{-123});

  // bin
  panicking::expect(Int<int>::from_str("0", 2), Option{0b0});
  panicking::expect(Int<int>::from_str("10", 2), Option{0b10});
  panicking::expect(Int<int>::from_str("-10", 2), Option{-0b10});

  // oct
  panicking::expect(Int<int>::from_str("0", 8), Option{0});
  panicking::expect(Int<int>::from_str("10", 8), Option{010});
  panicking::expect(Int<int>::from_str("-10", 8), Option{-010});

  // hex
  panicking::expect(Int<int>::from_str("0", 16), Option{0x0});
  panicking::expect(Int<int>::from_str("ff", 16), Option{0xff});
  panicking::expect(Int<int>::from_str("FF", 16), Option{0xFF});
  panicking::expect(Int<int>::from_str("-ff", 16), Option{-0xff});

  // auto-radix
  panicking::expect_eq(Int<int>::from_str("+0b101"), Option{+0b101});
  panicking::expect_eq(Int<int>::from_str("-0b101"), Option{-0b101});
  panicking::expect_eq(Int<int>::from_str("+010"), Option{+010});
  panicking::expect_eq(Int<int>::from_str("-010"), Option{-010});
  panicking::expect_eq(Int<int>::from_str("+0xFF"), Option{+0xFF});
  panicking::expect_eq(Int<int>::from_str("-0xFF"), Option{-0xFF});
}

}  // namespace sfc::num::test
