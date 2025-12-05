#include "sfc/test.h"

namespace sfc::str::test {

enum class E1 { A, B };

SFC_TEST(type_name) {
  panicking::expect_eq(str::type_name<int>(), "int");
  panicking::expect_eq(str::type_name<Str>(), "sfc::str::Str");
  panicking::expect_eq(str::type_name<E1>(), "sfc::str::test::E1");
}

SFC_TEST(simple) {
  {
    const auto s = Str{};
    panicking::expect_eq(s.len(), 0U);
    panicking::expect(s.is_empty());
    panicking::expect(!s);
  }

  {
    const auto s = Str{"abc"};
    panicking::expect_eq(s.as_ptr(), s._ptr);
    panicking::expect_eq(s.as_chars()._len, s._len);
    panicking::expect_eq(s.as_chars()._ptr, s._ptr);
    panicking::expect_eq(s.as_bytes()._len, s._len);
  }
}

SFC_TEST(index) {
  const auto s = Str{"abc"};
  panicking::expect_eq(s.len(), 3U);
  panicking::expect(s);
  panicking::expect(s.is_empty() == false);
  panicking::expect_eq(s[0], 'a');
  panicking::expect_eq(s[1], 'b');
  panicking::expect_eq(s[2], 'c');
  panicking::expect_eq(s[3], '\0');
}

SFC_TEST(cmp) {
  {
    const auto s = Str{"abc"};
    panicking::expect_eq(s, "abc");
    panicking::expect_ne(s, "ab");
    panicking::expect_ne(s, "abcd");
  }
}

SFC_TEST(range) {
  {
    const auto s = Str{"abc"};
    panicking::expect_eq(s[{0, 3}], "abc");
    panicking::expect_eq(s[{1, 3}], "bc");
    panicking::expect_eq(s[{0, 2}], "ab");
    panicking::expect_eq(s[{1, 2}], "b");
    panicking::expect_eq(s[{3, 3}], "");
    panicking::expect_eq(s[{4, 5}], "");

    panicking::expect_eq(s[{0, $}], "abc");
    panicking::expect_eq(s[{1, $}], "bc");
    panicking::expect_eq(s[{2, $}], "c");
    panicking::expect_eq(s[{3, $}], "");
  }
}

SFC_TEST(fmt) {
  panicking::expect_eq(string::format("{}", ""), "");
  panicking::expect_eq(string::format("{#}", ""), "");
  panicking::expect_eq(string::format("{?}", ""), "\"\"");
  panicking::expect_eq(string::format("{#?}", ""), "\"\"");

  panicking::expect_eq(string::format("{}", "abc"), "abc");
  panicking::expect_eq(string::format("{#}", "abc"), "abc");
  panicking::expect_eq(string::format("{?}", "abc"), "\"abc\"");
  panicking::expect_eq(string::format("{#?}", "abc"), "\"abc\"");
}

}  // namespace sfc::str::test
