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
    panicking::expect_true(s.is_empty());
    panicking::expect_false(s);
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
  panicking::expect_true(s);
  panicking::expect_true(s.is_empty() == false);
  panicking::expect_eq(s[0], 'a');
  panicking::expect_eq(s[1], 'b');
  panicking::expect_eq(s[2], 'c');
  panicking::expect_eq(s[3], '\0');
}

SFC_TEST(cmp) {
  {
    const auto s = Str{"abc"};
    panicking::expect_eq(s, Str{"abc"});
    panicking::expect_ne(s, Str{"ab"});
    panicking::expect_ne(s, Str{"abcd"});
  }
}

SFC_TEST(range) {
  {
    const auto s = Str{"abc"};
    panicking::expect_eq(s[{0, 3}], Str{"abc"});
    panicking::expect_eq(s[{1, 3}], Str{"bc"});
    panicking::expect_eq(s[{0, 2}], Str{"ab"});
    panicking::expect_eq(s[{1, 2}], Str{"b"});
    panicking::expect_eq(s[{3, 3}], Str{""});
    panicking::expect_eq(s[{4, 5}], Str{""});

    panicking::expect_eq(s[{0, $}], Str{"abc"});
    panicking::expect_eq(s[{1, $}], Str{"bc"});
    panicking::expect_eq(s[{2, $}], Str{"c"});
    panicking::expect_eq(s[{3, $}], Str{""});
  }
}

SFC_TEST(search_next) {
  const auto s = Str{"aabbcc"};

  {
    auto x = pattern::into_searcher('a', s);
    panicking::expect_eq(x.next(), Option<usize>{0});
    panicking::expect_eq(x.next(), Option<usize>{1});
    panicking::expect_false(x.next());
  }

  {
    auto x = pattern::into_searcher("ab", s);
    panicking::expect_false(x.next());
    panicking::expect_eq(x.next(), Option<usize>{1});
    panicking::expect_false(x.next());
  }
}

SFC_TEST(search_next_back) {
  const auto s = Str{"aabbcc"};

  {
    auto x = pattern::into_searcher('b', s);
    panicking::expect_false(x.next_back());
    panicking::expect_false(x.next_back());
    panicking::expect_eq(x.next_back(), Option<usize>{3});
    panicking::expect_eq(x.next_back(), Option<usize>{2});
    panicking::expect_false(x.next_back());
  }

  {
    auto x = pattern::into_searcher("bc", s);
    panicking::expect_false(x.next_back());
    panicking::expect_eq(x.next_back(), Option<usize>{3});
    panicking::expect_false(x.next_back());
  }
}

SFC_TEST(search_match) {
  const auto s = Str{"aabbcc"};

  {
    auto x = pattern::into_searcher('c', s);
    panicking::expect_eq(x.next_match(), Option<usize>{4});
    panicking::expect_eq(x.next_match(), Option<usize>{5});
    panicking::expect_false(x.next_match());
  }

  {
    auto x = pattern::into_searcher("ab", s);
    panicking::expect_eq(x.next_match(), Option<usize>{1});
    panicking::expect_false(x.next_match());
  }
}

SFC_TEST(find) {
  const auto s = Str{"abcabc"};
  panicking::expect_eq(s.find('a'), Option<usize>{0});
  panicking::expect_eq(s.find('b'), Option<usize>{1});
  panicking::expect_eq(s.find('c'), Option<usize>{2});
  panicking::expect_false(s.find('d'));

  panicking::expect_eq(s.find("ab"), Option<usize>{0});
  panicking::expect_eq(s.find("bc"), Option<usize>{1});
  panicking::expect_eq(s.find("ac"), Option<usize>{});
}

SFC_TEST(rfind) {
  const auto s = Str{"abcabc"};
  panicking::expect_eq(s.rfind('a'), Option<usize>{3});
  panicking::expect_eq(s.rfind('b'), Option<usize>{4});
  panicking::expect_eq(s.rfind('c'), Option<usize>{5});
  panicking::expect_false(s.rfind('d'));

  panicking::expect_eq(s.rfind("ab"), Option<usize>{3});
  panicking::expect_eq(s.rfind("bc"), Option<usize>{4});
  panicking::expect_eq(s.rfind("ac"), Option<usize>{});
}

SFC_TEST(contains) {
  const auto s = Str{"abc"};
  panicking::expect_false(s.contains('0'));
  panicking::expect_true(s.contains('a'));
  panicking::expect_true(s.contains('b'));

  panicking::expect_true(s.contains("ab"));
  panicking::expect_true(s.contains("abc"));
  panicking::expect_false(s.contains("abcd"));
}

SFC_TEST(starts_with) {
  const auto s = Str{"abc"};
  panicking::expect_true(s.starts_with('a'));
  panicking::expect_false(s.starts_with('b'));

  panicking::expect_true(s.starts_with("ab"));
  panicking::expect_true(s.starts_with("abc"));
  panicking::expect_false(s.starts_with("abcd"));

  panicking::expect_true(s.starts_with([](auto c) { return c == 'a'; }));
  panicking::expect_false(s.starts_with([](auto c) { return c == 'b'; }));
}

SFC_TEST(ends_with) {
  const auto s = Str{"abc"};
  panicking::expect_true(s.ends_with('c'));
  panicking::expect_false(s.ends_with('b'));

  panicking::expect_true(s.ends_with("bc"));
  panicking::expect_true(s.ends_with("abc"));
  panicking::expect_false(s.ends_with("abcd"));

  panicking::expect_true(s.ends_with([](auto c) { return c == 'c'; }));
  panicking::expect_false(s.ends_with([](auto c) { return c == 'b'; }));
}

SFC_TEST(trim) {
  {
    const auto s = Str{"   abc   "};
    panicking::expect_eq(s.trim_start(), Str{"abc   "});
    panicking::expect_eq(s.trim_end(), Str{"   abc"});
    panicking::expect_eq(s.trim(), Str{"abc"});
  }

  {
    const auto s = Str{"abc"};
    panicking::expect_eq(s.trim_start(), Str{"abc"});
    panicking::expect_eq(s.trim_end(), Str{"abc"});
    panicking::expect_eq(s.trim(), Str{"abc"});
  }
}

SFC_TEST(trim_matches) {
  {
    const auto s = Str{"   abc   "};
    panicking::expect_eq(s.trim_start_matches(' '), Str{"abc   "});
    panicking::expect_eq(s.trim_end_matches(' '), Str{"   abc"});
    panicking::expect_eq(s.trim_matches(' '), Str{"abc"});
  }

  {
    const auto s = Str{"abc"};
    panicking::expect_eq(s.trim_start_matches(' '), Str{"abc"});
    panicking::expect_eq(s.trim_end_matches(' '), Str{"abc"});
    panicking::expect_eq(s.trim_matches(' '), Str{"abc"});
  }
}

SFC_TEST(format) {
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
