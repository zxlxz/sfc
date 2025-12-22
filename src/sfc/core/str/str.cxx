#include "sfc/test/ut.h"

namespace sfc::str::test {

enum class E1 { A, B };

SFC_TEST(type_name) {
  panicking::expect_eq(str::type_name<int>(), "int");
  panicking::expect_eq(str::type_name<Str>(), "sfc::str::Str");
  panicking::expect_eq(str::type_name<E1>(), "sfc::str::test::E1");
}

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

SFC_TEST(find) {
  const auto s = Str{"ababcd"};

  // char
  panicking::expect_eq(s.find('a'), Option{0U});
  panicking::expect_eq(s.find('d'), Option{5U});
  panicking::expect_eq(s.find('x'), Option<usize>{});

  // str
  panicking::expect_eq(s.find("ab"), Option{0U});
  panicking::expect_eq(s.find("cd"), Option{4U});
  panicking::expect_eq(s.find(""), Option{0U});
  panicking::expect_eq(s.find("ababcd"), Option{0U});
  panicking::expect_eq(s.find("ababcdx"), Option<usize>{});

  // pred
  panicking::expect_eq(s.find([](char c) { return c == 'a'; }), Option{0U});
  panicking::expect_eq(s.find([](char c) { return c == 'd'; }), Option{5U});
  panicking::expect_eq(s.find([](char c) { return c == 'x'; }), Option<usize>{});
}

SFC_TEST(rfind) {
  const auto s = Str{"abcdcd"};

  // char
  panicking::expect_eq(s.rfind('a'), Option{0U});
  panicking::expect_eq(s.rfind('d'), Option{5U});
  panicking::expect_eq(s.rfind('x'), Option<usize>{});

  // str
  panicking::expect_eq(s.rfind("ab"), Option{0U});
  panicking::expect_eq(s.rfind("cd"), Option{4U});
  panicking::expect_eq(s.rfind(""), Option{6U});
  panicking::expect_eq(s.rfind("abcdcd"), Option{0U});
  panicking::expect_eq(s.rfind("xabcdcd"), Option<usize>{});

  // pred
  panicking::expect_eq(s.rfind([](char c) { return c == 'a'; }), Option{0U});
  panicking::expect_eq(s.rfind([](char c) { return c == 'd'; }), Option{5U});
  panicking::expect_eq(s.rfind([](char c) { return c == 'x'; }), Option<usize>{});
}

SFC_TEST(contains) {
  const auto s = Str{"ababcd"};

  // char
  panicking::expect(s.contains('a'));
  panicking::expect(!s.contains('x'));

  // str
  panicking::expect(s.contains("ab"));
  panicking::expect(!s.contains("xy"));

  // pred
  panicking::expect(s.contains([](char c) { return c == 'a'; }));
  panicking::expect(!s.contains([](char c) { return c == 'x'; }));
}

SFC_TEST(starts_with) {
  const auto s = Str{"ababcd"};

  // char
  panicking::expect(s.starts_with('a'));
  panicking::expect(!s.starts_with('b'));

  // str
  panicking::expect(s.starts_with("ab"));
  panicking::expect(!s.starts_with("bc"));

  // pred
  panicking::expect(s.starts_with([](char c) { return c == 'a'; }));
  panicking::expect(!s.starts_with([](char c) { return c == 'b'; }));
}

SFC_TEST(ends_with) {
  const auto s = Str{"ababcd"};

  // char
  panicking::expect(s.ends_with('d'));
  panicking::expect(!s.ends_with('c'));

  // str
  panicking::expect(s.ends_with("bcd"));
  panicking::expect(!s.ends_with("bc"));

  // pred
  panicking::expect(s.ends_with([](char c) { return c == 'd'; }));
  panicking::expect(!s.ends_with([](char c) { return c == 'c'; }));
}

SFC_TEST(trim) {
  const auto s1 = Str{"  \t\nabc  \n\t "};
  panicking::expect_eq(s1.trim_start(), "abc  \n\t ");
  panicking::expect_eq(s1.trim_end(), "  \t\nabc");
  panicking::expect_eq(s1.trim(), "abc");

  const auto s2 = Str{"xxxyabczyxx"};
  panicking::expect_eq(s2.trim_start_matches([](char c) { return c == 'x'; }), "yabczyxx");
  panicking::expect_eq(s2.trim_end_matches([](char c) { return c == 'x'; }), "xxxyabczy");
  panicking::expect_eq(s2.trim_matches([](char c) { return c == 'x' || c == 'y'; }), "abcz");
}

enum EE { A, B };

SFC_TEST(enum_name) {
  static_assert(str::type_name<EE>().len() > 0);
  panicking::expect_eq(str::type_name<EE>(), "sfc::str::test::EE");
  panicking::expect_eq(str::enum_name<EE::A>(), "A");
  panicking::expect_eq(str::enum_name<EE::B>(), "B");
}

}  // namespace sfc::str::test
