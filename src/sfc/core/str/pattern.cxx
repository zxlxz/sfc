#include "sfc/test/test.h"

namespace sfc::str::test {

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

}  // namespace sfc::str::test
