#include "sfc/test.h"

namespace sfc::fmt::test {

SFC_TEST(flt_fix) {
  panicking::expect_eq(string::format("{.0f}", +123.456), "123");
  panicking::expect_eq(string::format("{.0f}", -123.456), "-123");

  panicking::expect_eq(string::format("{.2f}", +123.456), "123.46");
  panicking::expect_eq(string::format("{.2f}", -123.456), "-123.46");

  panicking::expect_eq(string::format("{+.2f}", +123.456), "+123.46");
  panicking::expect_eq(string::format("{-.2f}", -123.456), "-123.46");

  panicking::expect_eq(string::format("{+8.2f}", +123.456), " +123.46");
  panicking::expect_eq(string::format("{+8.2f}", -123.456), " -123.46");

  panicking::expect_eq(string::format("{-8.2f}", +123.456), "  123.46");
  panicking::expect_eq(string::format("{-8.2f}", -123.456), " -123.46");
}

SFC_TEST(flt_exp) {
  panicking::expect_eq(string::format("{.2e}", +123.456), "1.23e+02");
  panicking::expect_eq(string::format("{.2e}", -123.456), "-1.23e+02");

  panicking::expect_eq(string::format("{+.2e}", +123.456), "+1.23e+02");
  panicking::expect_eq(string::format("{-.2e}", -123.456), "-1.23e+02");

  panicking::expect_eq(string::format("{+8.2e}", +123.456), "+1.23e+02");
  panicking::expect_eq(string::format("{+8.2e}", -123.456), "-1.23e+02");

  panicking::expect_eq(string::format("{-8.2e}", +123.456), " 1.23e+02");
  panicking::expect_eq(string::format("{-8.2e}", -123.456), "-1.23e+02");
}

}  // namespace sfc::fmt::test
