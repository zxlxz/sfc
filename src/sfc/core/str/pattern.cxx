#include "sfc/test/ut.h"

namespace sfc::str::test {

SFC_TEST(search_next) {
  const auto s = Str{"aabbcc"};

  {
    auto x = pattern::into_searcher('a', s);
    panicking::expect_eq(x.next(), Option<usize>{0});
    panicking::expect_eq(x.next(), Option<usize>{1});
    panicking::expect(!x.next());
  }

  {
    auto x = pattern::into_searcher("ab", s);
    panicking::expect(!x.next());
    panicking::expect_eq(x.next(), Option<usize>{1});
    panicking::expect(!x.next());
  }
}

SFC_TEST(search_next_back) {
  const auto s = Str{"aabbcc"};

  {
    auto x = pattern::into_searcher('b', s);
    panicking::expect(!x.next_back());
    panicking::expect(!x.next_back());
    panicking::expect_eq(x.next_back(), Option<usize>{3});
    panicking::expect_eq(x.next_back(), Option<usize>{2});
    panicking::expect(!x.next_back());
  }

  {
    auto x = pattern::into_searcher("bc", s);
    panicking::expect(!x.next_back());
    panicking::expect_eq(x.next_back(), Option<usize>{3});
    panicking::expect(!x.next_back());
  }
}

SFC_TEST(search_match) {
  const auto s = Str{"aabbcc"};

  {
    auto x = pattern::into_searcher('c', s);
    panicking::expect_eq(x.next_match(), Option<usize>{4});
    panicking::expect_eq(x.next_match(), Option<usize>{5});
    panicking::expect(!x.next_match());
  }

  {
    auto x = pattern::into_searcher("ab", s);
    panicking::expect_eq(x.next_match(), Option<usize>{1});
    panicking::expect(!x.next_match());
  }
}

SFC_TEST(find) {
  const auto s = Str{"abcabc"};
  panicking::expect_eq(s.find('a'), Option<usize>{0});
  panicking::expect_eq(s.find('b'), Option<usize>{1});
  panicking::expect_eq(s.find('c'), Option<usize>{2});
  panicking::expect(!s.find('d'));

  panicking::expect_eq(s.find("ab"), Option<usize>{0});
  panicking::expect_eq(s.find("bc"), Option<usize>{1});
  panicking::expect_eq(s.find("ac"), Option<usize>{});
}

SFC_TEST(rfind) {
  const auto s = Str{"abcabc"};
  panicking::expect_eq(s.rfind('a'), Option<usize>{3});
  panicking::expect_eq(s.rfind('b'), Option<usize>{4});
  panicking::expect_eq(s.rfind('c'), Option<usize>{5});
  panicking::expect(!s.rfind('d'));

  panicking::expect_eq(s.rfind("ab"), Option<usize>{3});
  panicking::expect_eq(s.rfind("bc"), Option<usize>{4});
  panicking::expect_eq(s.rfind("ac"), Option<usize>{});
}

SFC_TEST(contains) {
  const auto s = Str{"abc"};
  panicking::expect(!s.contains('0'));
  panicking::expect(s.contains('a'));
  panicking::expect(s.contains('b'));

  panicking::expect(s.contains("ab"));
  panicking::expect(s.contains("abc"));
  panicking::expect(!s.contains("abcd"));
}

SFC_TEST(starts_with) {
  const auto s = Str{"abc"};
  panicking::expect(s.starts_with('a'));
  panicking::expect(!s.starts_with('b'));

  panicking::expect(s.starts_with("ab"));
  panicking::expect(s.starts_with("abc"));
  panicking::expect(!s.starts_with("abcd"));

  panicking::expect(s.starts_with([](auto c) { return c == 'a'; }));
  panicking::expect(!s.starts_with([](auto c) { return c == 'b'; }));
}

SFC_TEST(ends_with) {
  const auto s = Str{"abc"};
  panicking::expect(s.ends_with('c'));
  panicking::expect(!s.ends_with('b'));

  panicking::expect(s.ends_with("bc"));
  panicking::expect(s.ends_with("abc"));
  panicking::expect(!s.ends_with("abcd"));

  panicking::expect(s.ends_with([](auto c) { return c == 'c'; }));
  panicking::expect(!s.ends_with([](auto c) { return c == 'b'; }));
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


}  // namespace sfc::str::test
