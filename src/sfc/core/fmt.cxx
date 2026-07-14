#include "sfc/test/test.h"

namespace sfc::fmt::test {

using string::format;

SFC_TEST(str) {
  sfc::assert_eq(format("[{:5}]", "x"), "[x    ]");
  sfc::assert_eq(format("[{:<5}]", "x"), "[x    ]");
  sfc::assert_eq(format("[{:^5}]", "x"), "[  x  ]");
  sfc::assert_eq(format("[{:>5}]", "x"), "[    x]");
}

SFC_TEST(num_sign) {
  sfc::assert_eq(format("{d}", 0), "0");
  sfc::assert_eq(format("{+d}", 0), "+0");
  sfc::assert_eq(format("{-d}", 0), " 0");

  sfc::assert_eq(format("{+}", 5), "+5");
  sfc::assert_eq(format("{-}", 5), " 5");

  sfc::assert_eq(format("{+}", -5), "-5");
  sfc::assert_eq(format("{-}", -5), "-5");
}

SFC_TEST(num_fill) {
  sfc::assert_eq(format("{3}", +5), "  5");
  sfc::assert_eq(format("{3}", -5), "  -5");

  sfc::assert_eq(format("{03}", +5), "005");
  sfc::assert_eq(format("{03}", -5), "-005");
}

SFC_TEST(num_alt) {
  sfc::assert_eq(format("{#x}", 5), "0x5");
  sfc::assert_eq(format("{#X}", 5), "0X5");
  sfc::assert_eq(format("{#b}", 5), "0b101");
  sfc::assert_eq(format("{#o}", 5), "05");

  sfc::assert_eq(format("{#2x}", 5), "0x05");
  sfc::assert_eq(format("{#2X}", 5), "0X05");
  sfc::assert_eq(format("{#3b}", 5), "0b101");
  sfc::assert_eq(format("{#3o}", 5), "0005");
}

SFC_TEST(num_align) {
  sfc::assert_eq(format("{>5d}", 42), "   42");
  sfc::assert_eq(format("{<5d}", 42), "42   ");
  sfc::assert_eq(format("{^5d}", 42), " 42  ");
  sfc::assert_eq(format("{=5d}", 42), "   42");

  sfc::assert_eq(format("{>4d}", -42), "  -42");
  sfc::assert_eq(format("{<4d}", -42), "-42  ");
  sfc::assert_eq(format("{^4d}", -42), " -42 ");
  sfc::assert_eq(format("{=4d}", -42), "-  42");
}

SFC_TEST(bool) {
  sfc::assert_eq(format("{}", true), "true");
  sfc::assert_eq(format("{}", false), "false");
}

SFC_TEST(char) {
  sfc::assert_eq(format("{}", 'a'), "a");
  sfc::assert_eq(format("{}", 'Z'), "Z");
}

SFC_TEST(ptr) {
  const void* p = nullptr;
  sfc::assert_eq(format("{}", p), "0x000000000000");
}

SFC_TEST(uint) {
  sfc::assert_eq(format("{}", 0U), "0");
  sfc::assert_eq(format("{}", 42U), "42");
  sfc::assert_eq(format("{}", 255U), "255");

  // max
  sfc::assert_eq(format("{}", u32{0xFFFFFFFFU}), "4294967295");
  sfc::assert_eq(format("{}", u64{0xFFFFFFFFFFFFFFFFULL}), "18446744073709551615");

  // hex
  sfc::assert_eq(format("{x}", 255U), "ff");
  sfc::assert_eq(format("{X}", 255U), "FF");

  // binary
  sfc::assert_eq(format("{b}", 5U), "101");
  sfc::assert_eq(format("{B}", 5U), "101");

  // octal
  sfc::assert_eq(format("{o}", 8U), "10");
  sfc::assert_eq(format("{O}", 8U), "10");
}

SFC_TEST(sint) {
  sfc::assert_eq(format("{}", 0), "0");
  sfc::assert_eq(format("{}", 42), "42");
  sfc::assert_eq(format("{}", -42), "-42");

  // min
  sfc::assert_eq(format("{}", num::Int<i32>::MIN), "-2147483648");
  sfc::assert_eq(format("{}", num::Int<i64>::MIN), "-9223372036854775808");

  // hex
  sfc::assert_eq(format("{x}", 255), "ff");
  sfc::assert_eq(format("{x}", -255), "-ff");

  // binary
  sfc::assert_eq(format("{b}", 5), "101");

  // octal
  sfc::assert_eq(format("{o}", 8), "10");
}

SFC_TEST(flt) {
  // f32 with default precision (4)
  sfc::assert_eq(format("{}", 1.0F), "1.0000");
  sfc::assert_eq(format("{}", 3.14159F), "3.1416");
  sfc::assert_eq(format("{}", -2.5F), "-2.5000");

  // f64 with default precision (6)
  sfc::assert_eq(format("{}", 1.0), "1.000000");
  sfc::assert_eq(format("{}", 3.14159), "3.141590");
  sfc::assert_eq(format("{}", -2.5), "-2.500000");

  // edge
  sfc::assert_eq(format("{.4}", 0.99994), "0.9999");
  sfc::assert_eq(format("{.4}", 0.99995), "1.0000");

  // precision
  sfc::assert_eq(format("{.2}", 3.14159), "3.14");
  sfc::assert_eq(format("{.0}", 3.14159), "3");
}

SFC_TEST(exp) {
  sfc::assert_eq(format("{e}", 1.0f), "1.0000e+00");
  sfc::assert_eq(format("{E}", 1.0f), "1.0000E+00");

  sfc::assert_eq(format("{e}", 1000.0f), "1.0000e+03");
  sfc::assert_eq(format("{e}", 0.001f), "1.0000e-03");

  sfc::assert_eq(format("{.2e}", 1234.5f), "1.23e+03");
  sfc::assert_eq(format("{.2e}", -1234.5f), "-1.23e+03");

  // edge
  sfc::assert_eq(format("{.3e}", 0.99994), "9.999e-01");
  sfc::assert_eq(format("{.3e}", 0.99995), "1.000e+00");
}

SFC_TEST(flt_nan) {
  sfc::assert_eq(format("{}", +__builtin_nan("")), "nan");
  sfc::assert_eq(format("{}", -__builtin_nan("")), "nan");

  sfc::assert_eq(format("{e}", +__builtin_nan("")), "nan");
  sfc::assert_eq(format("{e}", -__builtin_nan("")), "nan");
}

SFC_TEST(flt_inf) {
  static constexpr auto posinf = __builtin_bit_cast(f64, 0x7FF0000000000000);
  static constexpr auto neginf = __builtin_bit_cast(f64, 0xFFF0000000000000);

  sfc::assert_eq(format("{}", posinf), "inf");
  sfc::assert_eq(format("{}", neginf), "-inf");

  sfc::assert_eq(format("{e}", posinf), "inf");
  sfc::assert_eq(format("{e}", neginf), "-inf");
}

enum class E1 { A, B };
static auto to_str(E1 val) -> Str {
  switch (val) {
    case E1::A: return "A";
    case E1::B: return "B";
    default:    return "Unknown";
  }
}
SFC_TEST(named_enum) {
  sfc::assert_eq(reflect::type_name<E1>(), "sfc::fmt::test::E1");
  sfc::assert_eq(format("{}", E1::A), "A");
  sfc::assert_eq(format("{}", E1::B), "B");
}

enum E2 { X = -1, Y, Z };
SFC_TEST(unnamed_enum) {
  sfc::assert_eq(reflect::type_name<E2>(), "sfc::fmt::test::E2");

  sfc::assert_eq(format("{}", E2::X), "sfc::fmt::test::E2(-1)");
  sfc::assert_eq(format("{}", E2::Y), "sfc::fmt::test::E2(0)");
  sfc::assert_eq(format("{}", E2::Z), "sfc::fmt::test::E2(1)");
}

SFC_TEST(cstr) {
  sfc::assert_eq(format("{}", "hello"), "hello");
}

SFC_TEST(array) {
  int a[] = {1, 2, 3};
  sfc::assert_eq(format("{:?}", a), "[1, 2, 3]");

  char b[] = "abc";
  sfc::assert_eq(format("{:?}", b), "abc");
}

}  // namespace sfc::fmt::test
