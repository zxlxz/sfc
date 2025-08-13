#include "sfc/test.h"

namespace sfc::fmt::test {

SFC_TEST(display_bool) {
  panicking::assert_eq(string::format("{}", true), "true");
  panicking::assert_eq(string::format("{}", false), "false");

  panicking::assert_eq(string::format("{8}", true), "    true");
  panicking::assert_eq(string::format("{8}", false), "   false");

  panicking::assert_eq(string::format("{<8}", true), "true    ");
  panicking::assert_eq(string::format("{<8}", false), "false   ");

  panicking::assert_eq(string::format("{^8}", true), "  true  ");
  panicking::assert_eq(string::format("{^8}", false), " false  ");
}

}
