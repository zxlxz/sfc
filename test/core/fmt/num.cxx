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
  test::assert_eq(string::format("{}", s), "abc");
  test::assert_eq(string::format("{5}", s), "  abc");
  test::assert_eq(string::format("{<5}", s), "abc  ");
  test::assert_eq(string::format("{>5}", s), "  abc");
  test::assert_eq(string::format("{=5}", s), " abc ");
  test::assert_eq(string::format("{=5}", s), " abc ");
}

}  // namespace sfc::fmt
