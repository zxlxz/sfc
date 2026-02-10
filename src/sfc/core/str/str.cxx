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
  sfc::expect_eq(fmt::format("{}", ""), "");
  sfc::expect_eq(fmt::format("{#?}", ""), R"("")");

  sfc::expect_eq(fmt::format("{}", "abc"), "abc");
  sfc::expect_eq(fmt::format("{#?}", "abc"), R"("abc")");
}

SFC_TEST(find) {
  const auto s = Str{"ababcd"};

  // char
  sfc::expect_eq(s.find('a'), Option{0U});
  sfc::expect_eq(s.find('d'), Option{5U});
  sfc::expect_eq(s.find('x'), Option<usize>{});

  // str
  sfc::expect_eq(s.find("ab"), Option{0U});
  sfc::expect_eq(s.find("cd"), Option{4U});
  sfc::expect_eq(s.find(""), Option{0U});
  sfc::expect_eq(s.find("ababcd"), Option{0U});
  sfc::expect_eq(s.find("ababcdx"), Option<usize>{});

  // pred
  sfc::expect_eq(s.find([](char c) { return c == 'a'; }), Option{0U});
  sfc::expect_eq(s.find([](char c) { return c == 'd'; }), Option{5U});
  sfc::expect_eq(s.find([](char c) { return c == 'x'; }), Option<usize>{});
}

SFC_TEST(rfind) {
  const auto s = Str{"abcdcd"};

  // char
  sfc::expect_eq(s.rfind('a'), Option{0U});
  sfc::expect_eq(s.rfind('d'), Option{5U});
  sfc::expect_eq(s.rfind('x'), Option<usize>{});

  // str
  sfc::expect_eq(s.rfind("ab"), Option{0U});
  sfc::expect_eq(s.rfind("cd"), Option{4U});
  sfc::expect_eq(s.rfind(""), Option{6U});
  sfc::expect_eq(s.rfind("abcdcd"), Option{0U});
  sfc::expect_eq(s.rfind("xabcdcd"), Option<usize>{});

  // pred
  sfc::expect_eq(s.rfind([](char c) { return c == 'a'; }), Option{0U});
  sfc::expect_eq(s.rfind([](char c) { return c == 'd'; }), Option{5U});
  sfc::expect_eq(s.rfind([](char c) { return c == 'x'; }), Option<usize>{});
}

SFC_TEST(contains) {
  const auto s = Str{"ababcd"};

  // char
  sfc::expect_true(s.contains('a'));
  sfc::expect_false(s.contains('x'));

  // str
  sfc::expect_true(s.contains("ab"));
  sfc::expect_false(s.contains("xy"));

  // pred
  sfc::expect_true(s.contains([](char c) { return c == 'a'; }));
  sfc::expect_false(s.contains([](char c) { return c == 'x'; }));
}

SFC_TEST(starts_with) {
  const auto s = Str{"ababcd"};

  // char
  sfc::expect_true(s.starts_with('a'));
  sfc::expect_false(s.starts_with('b'));

  // str
  sfc::expect_true(s.starts_with("ab"));
  sfc::expect_false(s.starts_with("bc"));

  // pred
  sfc::expect_true(s.starts_with([](char c) { return c == 'a'; }));
  sfc::expect_false(s.starts_with([](char c) { return c == 'b'; }));
}

SFC_TEST(ends_with) {
  const auto s = Str{"ababcd"};

  // char
  sfc::expect_true(s.ends_with('d'));
  sfc::expect_false(s.ends_with('c'));

  // str
  sfc::expect_true(s.ends_with("bcd"));
  sfc::expect_false(s.ends_with("bc"));

  // pred
  sfc::expect_true(s.ends_with([](char c) { return c == 'd'; }));
  sfc::expect_false(s.ends_with([](char c) { return c == 'c'; }));
}

SFC_TEST(trim) {
  const auto s1 = Str{"  \t\nabc  \n\t "};
  sfc::expect_eq(s1.trim_start(), "abc  \n\t ");
  sfc::expect_eq(s1.trim_end(), "  \t\nabc");
  sfc::expect_eq(s1.trim(), "abc");

  const auto s2 = Str{"xxxyabczyxx"};
  sfc::expect_eq(s2.trim_start_matches([](char c) { return c == 'x'; }), "yabczyxx");
  sfc::expect_eq(s2.trim_end_matches([](char c) { return c == 'x'; }), "xxxyabczy");
  sfc::expect_eq(s2.trim_matches([](char c) { return c == 'x' || c == 'y'; }), "abcz");
}

}  // namespace sfc::str::test
