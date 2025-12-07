#include "sfc/test/ut.h"

namespace sfc::str::test {

enum class E1 { A, B };

SFC_TEST(type_name) {
  panicking::expect_eq(str::type_name<int>(), "int");
  panicking::expect_eq(str::type_name<Str>(), "sfc::str::Str");
  panicking::expect_eq(str::type_name<E1>(), "sfc::str::test::E1");
}

SFC_TEST(simple) {
  static_assert(Str{}.len() == 0U);
  static_assert(Str{"abc"}.len() == 3U);

  panicking::expect_eq(Str{}.len(), 0U);
  panicking::expect(Str{}.is_empty());
  panicking::expect(!Str{});

  const auto s = Str{"abc"};
  panicking::expect_eq(s.as_ptr(), s._ptr);
  panicking::expect_eq(s.as_chars()._len, s._len);
  panicking::expect_eq(s.as_chars()._ptr, s._ptr);
  panicking::expect_eq(s.as_bytes()._len, s._len);
}

SFC_TEST(index) {
  const auto s = Str{"01"};
  panicking::expect_eq(s[0], '0');
  panicking::expect_eq(s[1], '1');
  panicking::expect_eq(s[2], '\0');

  panicking::expect_eq(s[{0, 0}], "");
  panicking::expect_eq(s[{0, 2}], "01");
  panicking::expect_eq(s[{0, 3}], "01");

  panicking::expect_eq(s[{1, 2}], "1");
  panicking::expect_eq(s[{2, 2}], "");
  panicking::expect_eq(s[{3, 2}], "");

  panicking::expect_eq(s[{0, $}], "01");
  panicking::expect_eq(s[{1, $}], "1");
  panicking::expect_eq(s[{2, $}], "");
}

SFC_TEST(cmp) {
  const auto s = Str{"abc"};
  panicking::expect_eq(s, "abc");
  panicking::expect_ne(s, "ab");
  panicking::expect_ne(s, "abcd");
}

SFC_TEST(fmt) {
  panicking::expect_eq(string::format("{}", ""), "");
  panicking::expect_eq(string::format("{#?}", ""), R"("")");

  panicking::expect_eq(string::format("{}", "abc"), "abc");
  panicking::expect_eq(string::format("{#?}", "abc"), R"("abc")");
}

}  // namespace sfc::str::test
