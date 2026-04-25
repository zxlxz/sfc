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

  // empty str
  sfc::expect_eq(Str{}.find('a'), Option<usize>{});
  sfc::expect_eq(Str{}.find(""), Option<usize>{});
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

  // empty str
  sfc::expect_eq(Str{}.rfind('a'), Option<usize>{});
  sfc::expect_eq(Str{}.rfind(""), Option<usize>{});
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

  // empty str
  sfc::expect_eq(Str{}.contains('a'), false);
  sfc::expect_eq(Str{}.contains(""), false);
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

  // empty str
  sfc::expect_eq(Str{}.starts_with('a'), false);
  sfc::expect_eq(Str{}.starts_with(""), false);
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

  // empty str
  sfc::expect_eq(Str{}.ends_with('a'), false);
  sfc::expect_eq(Str{}.ends_with(""), false);
}

SFC_TEST(trim) {
  {
    const auto s = Str{"  \t\nabc  \n\t "};
    sfc::expect_eq(s.trim_start(), "abc  \n\t ");
    sfc::expect_eq(s.trim_end(), "  \t\nabc");
    sfc::expect_eq(s.trim(), "abc");
  }

  {
    const auto s = Str{"xxxyabczyxx"};
    sfc::expect_eq(s.trim_start_matches([](char c) { return c == 'x'; }), "yabczyxx");
    sfc::expect_eq(s.trim_end_matches([](char c) { return c == 'x'; }), "xxxyabczy");
    sfc::expect_eq(s.trim_matches([](char c) { return c == 'x' || c == 'y'; }), "abcz");
  }

  {
    const auto s = Str{};
    sfc::expect_eq(s.trim_start(), "");
    sfc::expect_eq(s.trim_end(), "");
    sfc::expect_eq(s.trim(), "");
  }
}


SFC_TEST(parse_uint) {
  sfc::expect_eq(Str{"123"}.parse<u64>(), Option<u64>{123U});
  sfc::expect_eq(Str{"0"}.parse<u64>(), Option<u64>{0U});
  sfc::expect_eq(Str{"4294967295"}.parse<u32>(), Option<u32>{0xFFFFFFFFU});
  sfc::expect_eq(Str{"0xff"}.parse<u64>(), Option<u64>{0xFFU});
  sfc::expect_eq(Str{"0xFF"}.parse<u64>(), Option<u64>{0xFFU});
  sfc::expect_eq(Str{"0b1010"}.parse<u64>(), Option<u64>{0xAU});
  sfc::expect_eq(Str{"0755"}.parse<u64>(), Option<u64>{0755U});

  // Invalid cases
  sfc::expect_eq(Str{""}.parse<u64>(), Option<u64>{});
  sfc::expect_eq(Str{"-123"}.parse<u64>(), Option<u64>{});
  sfc::expect_eq(Str{"abc"}.parse<u64>(), Option<u64>{});
}

SFC_TEST(parse_sint) {
  sfc::expect_eq(Str{"123"}.parse<i64>(), Option<i64>{123L});
  sfc::expect_eq(Str{"-123"}.parse<i64>(), Option<i64>{-123L});
  sfc::expect_eq(Str{"0"}.parse<i64>(), Option<i64>{0L});
  sfc::expect_eq(Str{"-2147483648"}.parse<i32>(), Option<i32>{-2147483648});
  sfc::expect_eq(Str{"2147483647"}.parse<i32>(), Option<i32>{2147483647});
  sfc::expect_eq(Str{"0xff"}.parse<i64>(), Option<i64>{0xFFL});
  sfc::expect_eq(Str{"-0xff"}.parse<i64>(), Option<i64>{-0xFFL});

  // Invalid cases
  sfc::expect_eq(Str{""}.parse<i64>(), Option<i64>{});
  sfc::expect_eq(Str{"abc"}.parse<i64>(), Option<i64>{});
}

SFC_TEST(parse_flt) {
  sfc::expect_flt_eq(Str{"3.14"}.parse<f64>().unwrap(), 3.14);
  sfc::expect_flt_eq(Str{"-3.14"}.parse<f64>().unwrap(), -3.14);
  sfc::expect_flt_eq(Str{"0.0"}.parse<f64>().unwrap(), 0.0);
  sfc::expect_flt_eq(Str{"123.456"}.parse<f64>().unwrap(), 123.456);
  sfc::expect_flt_eq(Str{"1.5"}.parse<f32>().unwrap(), 1.5F);

  // Invalid cases
  sfc::expect_eq(Str{""}.parse<f64>(), Option<f64>{});
  sfc::expect_eq(Str{"abc"}.parse<f64>(), Option<f64>{});
}

SFC_TEST(parse_exp) {
  sfc::expect_flt_eq(Str{"1e10"}.parse<f64>().unwrap(), 1e10);
  sfc::expect_flt_eq(Str{"1E10"}.parse<f64>().unwrap(), 1E10);
  sfc::expect_flt_eq(Str{"1.5e2"}.parse<f64>().unwrap(), 150.0);
  sfc::expect_flt_eq(Str{"1.5e-2"}.parse<f64>().unwrap(), 0.015);
  sfc::expect_flt_eq(Str{"-1.5e2"}.parse<f64>().unwrap(), -150.0);
  sfc::expect_flt_eq(Str{"-1.5e-2"}.parse<f64>().unwrap(), -0.015);
  sfc::expect_flt_eq(Str{"1e-10"}.parse<f64>().unwrap(), 1e-10);
}

}  // namespace sfc::str::test
