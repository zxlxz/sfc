#include "sfc/test.h"

namespace sfc::fmt {

SFC_TEST(int) {
  test::assert_eq(string::format("{}", +123), "123");
  test::assert_eq(string::format("{}", -123), "-123");

  test::assert_eq(string::format("{5}", +123), "123  ");
  test::assert_eq(string::format("{5}", -123), "-123 ");

  test::assert_eq(string::format("{<5}", +123), "123  ");
  test::assert_eq(string::format("{<5}", -123), "-123 ");

  test::assert_eq(string::format("{>5}", +123), "  123");
  test::assert_eq(string::format("{>5}", -123), " -123");

  test::assert_eq(string::format("{=6}", +123), "   123");
  test::assert_eq(string::format("{=6}", -123), "-  123");

  test::assert_eq(string::format("{^6}", +123), " 123  ");
  test::assert_eq(string::format("{^6}", -123), " -123 ");

  test::assert_eq(string::format("{06}", +123), "000123");
  test::assert_eq(string::format("{06}", -123), "-00123");

  test::assert_eq(string::format("{+06}", +123), "+00123");
  test::assert_eq(string::format("{+06}", -123), "-00123");

  test::assert_eq(string::format("{-06}", +123), " 00123");
  test::assert_eq(string::format("{-06}", -123), "-00123");
}

SFC_TEST(flt) {
  test::assert_eq(string::format("{}", +12.34), "12.340000");
  test::assert_eq(string::format("{}", -12.34), "-12.340000");

  test::assert_eq(string::format("{.3}", +12.34), "12.340");
  test::assert_eq(string::format("{.3}", -12.34), "-12.340");

  test::assert_eq(string::format("{8.3}", +12.34), "12.340  ");
  test::assert_eq(string::format("{8.3}", -12.34), "-12.340 ");

  test::assert_eq(string::format("{+.3}", +12.34), "+12.340");
  test::assert_eq(string::format("{+.3}", -12.34), "-12.340");

  test::assert_eq(string::format("{-.3}", +12.34), " 12.340");
  test::assert_eq(string::format("{-.3}", -12.34), "-12.340");

  test::assert_eq(string::format("{<8.3}", +12.34), "12.340  ");
  test::assert_eq(string::format("{<8.3}", -12.34), "-12.340 ");

  test::assert_eq(string::format("{>8.3}", +12.34), "  12.340");
  test::assert_eq(string::format("{>8.3}", -12.34), " -12.340");

  test::assert_eq(string::format("{^8.3}", +12.34), " 12.340 ");
  test::assert_eq(string::format("{^8.3}", -12.34), "-12.340 ");

  test::assert_eq(string::format("{=8.3}", +12.34), "  12.340");
  test::assert_eq(string::format("{=8.3}", -12.34), "- 12.340");
}

SFC_TEST(ptr) {
  const char* s = "abc";
  const auto ns = string::format("0x{x}", mem::bit_cast<u64>(s));

  test::assert_eq(string::format("{}", mem::bit_cast<const int*>(s)), ns);
  test::assert_eq(string::format("{}", mem::bit_cast<const u8*>(s)), ns);
  test::assert_eq(string::format("{}", mem::bit_cast<const i8*>(s)), ns);

  test::assert_eq(string::format("{}", static_cast<const char*>(s)), "abc");
  test::assert_eq(string::format("{}", const_cast<char*>(s)), "abc");
}

}  // namespace sfc::fmt
