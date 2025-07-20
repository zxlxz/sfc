#include "sfc/test.h"

namespace sfc::fmt::test {

SFC_TEST(int) {
  panicking::assert_eq(string::format("{}", +123), "123");
  panicking::assert_eq(string::format("{}", -123), "-123");

  panicking::assert_eq(string::format("{5}", +123), "  123");
  panicking::assert_eq(string::format("{5}", -123), " -123");

  panicking::assert_eq(string::format("{<5}", +123), "123  ");
  panicking::assert_eq(string::format("{<5}", -123), "-123 ");

  panicking::assert_eq(string::format("{>5}", +123), "  123");
  panicking::assert_eq(string::format("{>5}", -123), " -123");

  panicking::assert_eq(string::format("{=6}", +123), "   123");
  panicking::assert_eq(string::format("{=6}", -123), "-  123");

  panicking::assert_eq(string::format("{^6}", +123), " 123  ");
  panicking::assert_eq(string::format("{^6}", -123), " -123 ");

  panicking::assert_eq(string::format("{06}", +123), "000123");
  panicking::assert_eq(string::format("{06}", -123), "-00123");

  panicking::assert_eq(string::format("{+06}", +123), "+00123");
  panicking::assert_eq(string::format("{+06}", -123), "-00123");

  panicking::assert_eq(string::format("{-06}", +123), " 00123");
  panicking::assert_eq(string::format("{-06}", -123), "-00123");
}

SFC_TEST(flt) {
  panicking::assert_eq(string::format("{}", +12.34), "12.340000");
  panicking::assert_eq(string::format("{}", -12.34), "-12.340000");

  panicking::assert_eq(string::format("{.3}", +12.34), "12.340");
  panicking::assert_eq(string::format("{.3}", -12.34), "-12.340");

  panicking::assert_eq(string::format("{8.3}", +12.34), "  12.340");
  panicking::assert_eq(string::format("{8.3}", -12.34), " -12.340");

  panicking::assert_eq(string::format("{+.3}", +12.34), "+12.340");
  panicking::assert_eq(string::format("{+.3}", -12.34), "-12.340");

  panicking::assert_eq(string::format("{-.3}", +12.34), " 12.340");
  panicking::assert_eq(string::format("{-.3}", -12.34), "-12.340");

  panicking::assert_eq(string::format("{<8.3}", +12.34), "12.340  ");
  panicking::assert_eq(string::format("{<8.3}", -12.34), "-12.340 ");

  panicking::assert_eq(string::format("{>8.3}", +12.34), "  12.340");
  panicking::assert_eq(string::format("{>8.3}", -12.34), " -12.340");

  panicking::assert_eq(string::format("{^8.3}", +12.34), " 12.340 ");
  panicking::assert_eq(string::format("{^8.3}", -12.34), "-12.340 ");

  panicking::assert_eq(string::format("{=8.3}", +12.34), "  12.340");
  panicking::assert_eq(string::format("{=8.3}", -12.34), "- 12.340");
}

SFC_TEST(ptr) {
  const char* s = "abc";
  panicking::assert_eq(string::format("{}", s), "abc");
  panicking::assert_eq(string::format("{5}", s), "  abc");
  panicking::assert_eq(string::format("{<5}", s), "abc  ");
  panicking::assert_eq(string::format("{>5}", s), "  abc");
  panicking::assert_eq(string::format("{=5}", s), " abc ");
  panicking::assert_eq(string::format("{=5}", s), " abc ");
}

}  // namespace sfc::fmt::test
