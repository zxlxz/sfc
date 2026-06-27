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
  sfc::assert_eq(s[0], '0');
  sfc::assert_eq(s[1], '1');
  sfc::assert_eq(s[2], '\0');

  sfc::assert_eq(s[{0, 0}], "");
  sfc::assert_eq(s[{0, 2}], "01");
  sfc::assert_eq(s[{0, 3}], "01");

  sfc::assert_eq(s[{1, 2}], "1");
  sfc::assert_eq(s[{2, 2}], "");
  sfc::assert_eq(s[{3, 2}], "");

  sfc::assert_eq(s[{0, $}], "01");
  sfc::assert_eq(s[{1, $}], "1");
  sfc::assert_eq(s[{2, $}], "");
}

SFC_TEST(cmp) {
  const auto s = Str{"abc"};
  sfc::assert_eq(s, "abc");
  sfc::assert_ne(s, "ab");
  sfc::assert_ne(s, "abcd");
}

SFC_TEST(fmt) {
  sfc::assert_eq(string::format("{}", ""), "");
  sfc::assert_eq(string::format("{#?}", Str{""}), R"("")");

  sfc::assert_eq(string::format("{}", "abc"), "abc");
  sfc::assert_eq(string::format("{#?}", Str{"abc"}), R"("abc")");
}

SFC_TEST(find) {
  const auto s = Str{"ababcd"};

  // char
  sfc::assert_eq(s.find('a'), Option{0UZ});
  sfc::assert_eq(s.find('d'), Option{5UZ});
  sfc::assert_eq(s.find('x'), Option{});

  // str
  sfc::assert_eq(s.find("ab"), Option{0UZ});
  sfc::assert_eq(s.find("cd"), Option{4UZ});
  sfc::assert_eq(s.find(""), Option{0UZ});
  sfc::assert_eq(s.find("ababcd"), Option{0UZ});
  sfc::assert_eq(s.find("ababcdx"), Option{});

  // pred
  sfc::assert_eq(s.find([](char c) { return c == 'a'; }), Option{0UZ});
  sfc::assert_eq(s.find([](char c) { return c == 'd'; }), Option{5UZ});
  sfc::assert_eq(s.find([](char c) { return c == 'x'; }), Option{});

  // empty str
  sfc::assert_eq(Str{}.find('a'), Option{});
  sfc::assert_eq(Str{}.find(""), Option{});
}

SFC_TEST(rfind) {
  const auto s = Str{"abcdcd"};

  // char
  sfc::assert_eq(s.rfind('a'), Option{0UZ});
  sfc::assert_eq(s.rfind('d'), Option{5UZ});
  sfc::assert_eq(s.rfind('x'), Option{});

  // str
  sfc::assert_eq(s.rfind("ab"), Option{0UZ});
  sfc::assert_eq(s.rfind("cd"), Option{4UZ});
  sfc::assert_eq(s.rfind(""), Option{6UZ});
  sfc::assert_eq(s.rfind("abcdcd"), Option{0UZ});
  sfc::assert_eq(s.rfind("xabcdcd"), Option{});

  // pred
  sfc::assert_eq(s.rfind([](char c) { return c == 'a'; }), Option{0UZ});
  sfc::assert_eq(s.rfind([](char c) { return c == 'd'; }), Option{5UZ});
  sfc::assert_eq(s.rfind([](char c) { return c == 'x'; }), Option{});

  // empty str
  sfc::assert_eq(Str{}.rfind('a'), Option{});
  sfc::assert_eq(Str{}.rfind(""), Option{});
}

SFC_TEST(contains) {
  const auto s = Str{"ababcd"};

  // char
  sfc::assert_eq(s.contains('a'), true);
  sfc::assert_eq(s.contains('x'), false);

  // str
  sfc::assert_eq(s.contains("ab"), true);
  sfc::assert_eq(s.contains("xy"), false);

  // pred
  sfc::assert_eq(s.contains([](char c) { return c == 'a'; }), true);
  sfc::assert_eq(s.contains([](char c) { return c == 'x'; }), false);

  // empty str
  sfc::assert_eq(Str{}.contains('a'), false);
  sfc::assert_eq(Str{}.contains(""), false);
}

SFC_TEST(starts_with) {
  const auto s = Str{"ababcd"};

  // char
  sfc::assert_eq(s.starts_with('a'), true);
  sfc::assert_eq(s.starts_with('b'), false);

  // str
  sfc::assert_eq(s.starts_with("ab"), true);
  sfc::assert_eq(s.starts_with("bc"), false);

  // pred
  sfc::assert_eq(s.starts_with([](char c) { return c == 'a'; }), true);
  sfc::assert_eq(s.starts_with([](char c) { return c == 'b'; }), false);

  // empty str
  sfc::assert_eq(Str{}.starts_with('a'), false);
  sfc::assert_eq(Str{}.starts_with(""), false);
}

SFC_TEST(ends_with) {
  const auto s = Str{"ababcd"};

  // char
  sfc::assert_eq(s.ends_with('d'), true);
  sfc::assert_eq(s.ends_with('c'), false);

  // str
  sfc::assert_eq(s.ends_with("bcd"), true);
  sfc::assert_eq(s.ends_with("bc"), false);

  // pred
  sfc::assert_eq(s.ends_with([](char c) { return c == 'd'; }), true);
  sfc::assert_eq(s.ends_with([](char c) { return c == 'c'; }), false);

  // empty str
  sfc::assert_eq(Str{}.ends_with('a'), false);
  sfc::assert_eq(Str{}.ends_with(""), false);
}

SFC_TEST(trim) {
  {
    const auto s = Str{"  \t\nabc  \n\t "};
    sfc::assert_eq(s.trim_start(), "abc  \n\t ");
    sfc::assert_eq(s.trim_end(), "  \t\nabc");
    sfc::assert_eq(s.trim(), "abc");
  }

  {
    const auto s = Str{"xxxyabczyxx"};
    sfc::assert_eq(s.trim_start_matches([](char c) { return c == 'x'; }), "yabczyxx");
    sfc::assert_eq(s.trim_end_matches([](char c) { return c == 'x'; }), "xxxyabczy");
    sfc::assert_eq(s.trim_matches([](char c) { return c == 'x' || c == 'y'; }), "abcz");
  }

  {
    const auto s = Str{};
    sfc::assert_eq(s.trim_start(), "");
    sfc::assert_eq(s.trim_end(), "");
    sfc::assert_eq(s.trim(), "");
  }
}

SFC_TEST(parse_uint) {
  // u32
  sfc::assert_eq(Str{"0"}.parse<u64>(), Option{0U});
  sfc::assert_eq(Str{"123"}.parse<u64>(), Option{123U});
  sfc::assert_eq(Str{"4294967295"}.parse<u32>(), Option{0xFFFFFFFFU});

  // hex
  sfc::assert_eq(Str{"0xff"}.parse<u64>(), Option{0xFFU});
  sfc::assert_eq(Str{"0xFF"}.parse<u64>(), Option{0xFFU});
  sfc::assert_eq(Str{"0b1010"}.parse<u64>(), Option{0xAU});
  sfc::assert_eq(Str{"0755"}.parse<u64>(), Option{0755U});

  // Invalid cases
  sfc::assert_eq(Str{""}.parse<u64>(), Option{});
  sfc::assert_eq(Str{"-123"}.parse<u64>(), Option{});
  sfc::assert_eq(Str{"abc"}.parse<u64>(), Option{});
}

SFC_TEST(parse_sint) {
  // i32
  sfc::assert_eq(Str{"0"}.parse<i32>(), Option{0L});
  sfc::assert_eq(Str{"123"}.parse<i32>(), Option{123L});
  sfc::assert_eq(Str{"-123"}.parse<i32>(), Option{-123L});
  sfc::assert_eq(Str{"-2147483648"}.parse<i32>(), Option{-2147483648});
  sfc::assert_eq(Str{"2147483647"}.parse<i32>(), Option{2147483647});

  // i32 overflow
  sfc::assert_eq(Str{"-21474836481"}.parse<i32>(), Option{});
  sfc::assert_eq(Str{"21474836471"}.parse<i32>(), Option{});

  // i64
  sfc::assert_eq(Str{"-21474836481"}.parse<i64>(), Option{-21474836481});
  sfc::assert_eq(Str{"21474836471"}.parse<i64>(), Option{21474836471});

  // hex
  sfc::assert_eq(Str{"0xff"}.parse<i64>(), Option{0xFFL});
  sfc::assert_eq(Str{"-0xff"}.parse<i64>(), Option{-0xFFL});

  // Invalid cases
  sfc::assert_eq(Str{""}.parse<i64>(), Option{});
  sfc::assert_eq(Str{"abc"}.parse<i64>(), Option{});
}

SFC_TEST(parse_flt) {
  sfc::assert_eq(Str{"3.14"}.parse<f64>(), Option{3.14});
  sfc::assert_eq(Str{"-3.14"}.parse<f64>(), Option{-3.14});
  sfc::assert_eq(Str{"0.0"}.parse<f64>(), Option{0.0});
  sfc::assert_eq(Str{"123.456"}.parse<f64>(), Option{123.456});
  sfc::assert_eq(Str{"1.5"}.parse<f32>(), Option{1.5F});

  // Invalid cases
  sfc::assert_eq(Str{""}.parse<f64>(), Option{});
  sfc::assert_eq(Str{"abc"}.parse<f64>(), Option{});
}

SFC_TEST(parse_exp) {
  sfc::assert_eq(Str{"1e10"}.parse<f64>(), Option{1e10});
  sfc::assert_eq(Str{"1E10"}.parse<f64>(), Option{1E10});
  sfc::assert_eq(Str{"1.5e2"}.parse<f64>(), Option{150.0});
  sfc::assert_eq(Str{"1.5e-2"}.parse<f64>(), Option{0.015});
  sfc::assert_eq(Str{"-1.5e2"}.parse<f64>(), Option{-150.0});
  sfc::assert_eq(Str{"-1.5e-2"}.parse<f64>(), Option{-0.015});
  sfc::assert_eq(Str{"1e-10"}.parse<f64>(), Option{1e-10});
}

SFC_TEST(split_once) {
  const auto s = Str{"a,b;c"};
  {
    const auto [a, b] = s.split_once(',').unwrap_or({{}, {}});
    sfc::assert_eq(a, "a");
    sfc::assert_eq(b, "b;c");
  }

  {
    const auto [a, b] = s.split_once(';').unwrap_or({{}, {}});
    sfc::assert_eq(a, "a,b");
    sfc::assert_eq(b, "c");
  }
}

}  // namespace sfc::str::test
