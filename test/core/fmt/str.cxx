#include "sfc/test.h"

namespace sfc::fmt {

SFC_TEST(str) {
  panicking::assert_eq(string::format("{}", "abc"), "abc");

  panicking::assert_eq(string::format("{5}", "abc"), "  abc");
  panicking::assert_eq(string::format("{>5}", "abc"), "  abc");
  panicking::assert_eq(string::format("{<5}", "abc"), "abc  ");
  panicking::assert_eq(string::format("{=5}", "abc"), " abc ");
  panicking::assert_eq(string::format("{^5}", "abc"), " abc ");
}

}  // namespace sfc::fmt
