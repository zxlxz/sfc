#include "sfc/test/test.h"

namespace sfc::fmt::test {

using string::format;

SFC_TEST(bool) {
  sfc::expect_eq(format("{}", true), "true");
  sfc::expect_eq(format("{}", false), "false");
}

SFC_TEST(char) {
  sfc::expect_eq(format("{}", 'a'), "a");
  sfc::expect_eq(format("{}", 'Z'), "Z");
}

SFC_TEST(ptr) {
  sfc::expect_eq(format("{}", nullptr), "0x000000000000");
}

SFC_TEST(uint) {
  sfc::expect_eq(format("{}", 0U), "0");
  sfc::expect_eq(format("{}", 42U), "42");
  sfc::expect_eq(format("{}", 255U), "255");

  // hex
  sfc::expect_eq(format("{x}", 255U), "ff");
  sfc::expect_eq(format("{X}", 255U), "FF");

  // binary
  sfc::expect_eq(format("{b}", 5U), "101");
  sfc::expect_eq(format("{B}", 5U), "101");

  // octal
  sfc::expect_eq(format("{o}", 8U), "10");
  sfc::expect_eq(format("{O}", 8U), "10");
}

SFC_TEST(sint) {
  sfc::expect_eq(format("{}", 0), "0");
  sfc::expect_eq(format("{}", 42), "42");
  sfc::expect_eq(format("{}", -42), "-42");

  // hex
  sfc::expect_eq(format("{x}", 255), "ff");
  sfc::expect_eq(format("{x}", -255), "-ff");

  // binary
  sfc::expect_eq(format("{b}", 5), "101");

  // octal
  sfc::expect_eq(format("{o}", 8), "10");
}

SFC_TEST(flt) {
  // f32 with default precision (4)
  sfc::expect_eq(format("{}", 1.0F), "1.0000");
  sfc::expect_eq(format("{}", 3.14159F), "3.1416");
  sfc::expect_eq(format("{}", -2.5F), "-2.5000");

  // f64 with default precision (6)
  sfc::expect_eq(format("{}", 1.0), "1.000000");
  sfc::expect_eq(format("{}", 3.14159), "3.141590");
  sfc::expect_eq(format("{}", -2.5), "-2.500000");

  // precision
  sfc::expect_eq(format("{.2}", 3.14159), "3.14");
  sfc::expect_eq(format("{.0}", 3.14159), "3");
}

SFC_TEST(exp) {
  sfc::expect_eq(format("{e}", 1.0f), "1.0000e+00");
  sfc::expect_eq(format("{E}", 1.0f), "1.0000E+00");

  sfc::expect_eq(format("{e}", 1000.0f), "1.0000e+03");
  sfc::expect_eq(format("{e}", 0.001f), "1.0000e-03");

  sfc::expect_eq(format("{.2e}", 1234.5f), "1.23e+03");
}

SFC_TEST(flt_nan) {
  sfc::expect_eq(format("{}", +__builtin_nan("")), "nan");
  sfc::expect_eq(format("{}", -__builtin_nan("")), "nan");

  sfc::expect_eq(format("{e}", +__builtin_nan("")), "nan");
  sfc::expect_eq(format("{e}", -__builtin_nan("")), "nan");
}

SFC_TEST(flt_inf) {
  sfc::expect_eq(format("{}", +__builtin_inf()), "inf");
  sfc::expect_eq(format("{}", -__builtin_inf()), "-inf");

  sfc::expect_eq(format("{e}", +__builtin_inf()), "inf");
  sfc::expect_eq(format("{e}", -__builtin_inf()), "-inf");
}

enum class E1 { A, B };
SFC_TEST(named_enum) {
  sfc::expect_eq(reflect::type_name<E1>(), "sfc::fmt::test::E1");
  sfc::expect_eq(format("{}", E1::A), "A");
  sfc::expect_eq(format("{}", E1::B), "B");
}

enum E2 { X = -1, Y, Z };
SFC_TEST(unnamed_enum) {
  sfc::expect_eq(reflect::type_name<E2>(), "sfc::fmt::test::E2");
  sfc::expect_eq(format("{}", E2::X), "sfc::fmt::test::E2(-1)");
  sfc::expect_eq(format("{}", E2::Y), "sfc::fmt::test::E2(0)");
  sfc::expect_eq(format("{}", E2::Z), "sfc::fmt::test::E2(1)");
}

SFC_TEST(cstr) {
  sfc::expect_eq(format("{}", "hello"), "hello");
}

}  // namespace sfc::fmt::test
