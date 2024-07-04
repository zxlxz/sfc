#include "sfc/test.h"

namespace sfc::fmt {

SFC_TEST(str) {
  test::assert_eq(string::format("{}", "abc"), "abc");

  test::assert_eq(string::format("{5}", "abc"), "abc  ");
  test::assert_eq(string::format("{>5}", "abc"), "  abc");
  test::assert_eq(string::format("{<5}", "abc"), "abc  ");
  test::assert_eq(string::format("{=5}", "abc"), " abc ");
  test::assert_eq(string::format("{^5}", "abc"), " abc ");
}

}  // namespace sfc::fmt
