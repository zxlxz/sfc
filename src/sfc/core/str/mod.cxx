#include "sfc/test/test.h"

namespace sfc::str::test {

SFC_TEST(constexpr) {
  // is_empty
  static_assert(Str{}.is_empty());
  static_assert(!Str{"abc"}.is_empty());

  // len
  static_assert(Str{}.len() == 0U);
  static_assert(Str{"abc"}.len() == 3U);

  // idx
  static_assert(Str{"abc"}[0] == 'a');
  static_assert(Str{"abc"}[4] == '\0');
}

SFC_TEST(index) {
  const auto s = Str{"01"};
  sfc::expect_eq(s[0], '0');
  sfc::expect_eq(s[1], '1');
  sfc::expect_eq(s[2], '\0');

  sfc::expect_eq(s[{0, 0}], "");
  sfc::expect_eq(s[{0, 2}], "01");
  sfc::expect_eq(s[{0, 3}], "01");

  sfc::expect_eq(s[{1, 2}], "1");
  sfc::expect_eq(s[{2, 2}], "");
  sfc::expect_eq(s[{3, 2}], "");

  sfc::expect_eq(s[{0, $}], "01");
  sfc::expect_eq(s[{1, $}], "1");
  sfc::expect_eq(s[{2, $}], "");
}

SFC_TEST(cmp) {
  const auto s = Str{"abc"};
  sfc::expect_eq(s, "abc");
  sfc::expect_ne(s, "ab");
  sfc::expect_ne(s, "abcd");
}

SFC_TEST(fmt) {
  sfc::expect_eq(string::format("{}", ""), "");
  sfc::expect_eq(string::format("{#?}", Str{""}), R"("")");

  sfc::expect_eq(string::format("{}", "abc"), "abc");
  sfc::expect_eq(string::format("{#?}", Str{"abc"}), R"("abc")");
}

}  // namespace sfc::str::test
