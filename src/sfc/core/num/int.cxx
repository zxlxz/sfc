#include "sfc/test/test.h"

namespace sfc::num::test {

SFC_TEST(int_to_str) {
  char buf[32] = {};

  // dec
  sfc::expect_eq(Int{0}.to_str(buf), "0");
  sfc::expect_eq(Int{+123}.to_str(buf), "123");
  sfc::expect_eq(Int{-123}.to_str(buf), "-123");
}

SFC_TEST(bin_to_str) {
  char buf[32] = {};

  // bin
  sfc::expect_eq(Int{0}.to_str(buf, 'b'), "0");

  sfc::expect_eq(Int{+5}.to_str(buf, 'b'), "101");
  sfc::expect_eq(Int{+5}.to_str(buf, 'B'), "101");
  sfc::expect_eq(Int{-5}.to_str(buf, 'b'), "-101");
  sfc::expect_eq(Int{-5}.to_str(buf, 'B'), "-101");

  // oct
  sfc::expect_eq(Int{0}.to_str(buf, 'o'), "0");
  sfc::expect_eq(Int{+8}.to_str(buf, 'o'), "10");
  sfc::expect_eq(Int{-8}.to_str(buf, 'o'), "-10");

  // hex
  sfc::expect_eq(Int{0}.to_str(buf, 'x'), "0");
  sfc::expect_eq(Int{255}.to_str(buf, 'x'), "ff");
  sfc::expect_eq(Int{255}.to_str(buf, 'X'), "FF");
  sfc::expect_eq(Int{-255}.to_str(buf, 'x'), "-ff");
  sfc::expect_eq(Int{-255}.to_str(buf, 'X'), "-FF");
}

SFC_TEST(int_from_str) {
  // dec
  sfc::expect(Int<int>::from_str("0"), Option{0});
  sfc::expect(Int<int>::from_str("+123"), Option{123});
  sfc::expect(Int<int>::from_str("-123"), Option{-123});

  // neg
  sfc::expect_eq(Int<i8>::from_str("-128"), Option<i8>{-128});
  sfc::expect_eq(Int<i16>::from_str("-32768"), Option<i16>{-32768});

  // uint
  sfc::expect_eq(Int<u8>::from_str("-1"), Option<u8>{});
  sfc::expect_eq(Int<u32>::from_str("-1"), Option<u32>{});
  sfc::expect_eq(Int<u32>::from_str("+1"), Option<u32>{1});

  // unsigned overflow
  sfc::expect_eq(Int<u8>::from_str("255"), Option<u8>{255});
  sfc::expect_eq(Int<u8>::from_str("256"), Option<u8>{});

  // signed overflow
  sfc::expect_eq(Int<i8>::from_str("-128"), Option<i8>{-128});
  sfc::expect_eq(Int<i8>::from_str("128"), Option<i8>{});
}

SFC_TEST(bin_from_str) {
  // dec
  sfc::expect(Int<int>::from_str("0"), Option{0});
  sfc::expect(Int<int>::from_str("+123"), Option{123});
  sfc::expect(Int<int>::from_str("-123"), Option{-123});

  // bin
  sfc::expect(Int<int>::from_str("0", 2), Option{0b0});
  sfc::expect(Int<int>::from_str("10", 2), Option{0b10});
  sfc::expect(Int<int>::from_str("-10", 2), Option{-0b10});

  // oct
  sfc::expect(Int<int>::from_str("0", 8), Option{0});
  sfc::expect(Int<int>::from_str("10", 8), Option{010});
  sfc::expect(Int<int>::from_str("-10", 8), Option{-010});

  // hex
  sfc::expect(Int<int>::from_str("0", 16), Option{0x0});
  sfc::expect(Int<int>::from_str("ff", 16), Option{0xff});
  sfc::expect(Int<int>::from_str("FF", 16), Option{0xFF});
  sfc::expect(Int<int>::from_str("-ff", 16), Option{-0xff});

  // auto-radix
  sfc::expect_eq(Int<int>::from_str("+0b101"), Option{+0b101});
  sfc::expect_eq(Int<int>::from_str("-0b101"), Option{-0b101});
  sfc::expect_eq(Int<int>::from_str("+010"), Option{+010});
  sfc::expect_eq(Int<int>::from_str("-010"), Option{-010});
  sfc::expect_eq(Int<int>::from_str("+0xFF"), Option{+0xFF});
  sfc::expect_eq(Int<int>::from_str("-0xFF"), Option{-0xFF});
}

}  // namespace sfc::num::test
