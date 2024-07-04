#include "sfc/core.h"
#include "sfc/test.h"

namespace sfc::str {

SFC_TEST(simple) {
  {
    const auto s = Str{};
    panicking::assert_eq(s.len(), 0U);
    panicking::assert_true(s.is_empty());
    panicking::assert_false(s);
  }

  {
    const auto s = Str{"abc"};
    panicking::assert_eq(s.as_ptr(), s._ptr);
    panicking::assert_eq(s.as_chars()._len, s._len);
    panicking::assert_eq(s.as_chars()._ptr, s._ptr);
    panicking::assert_eq(s.as_bytes()._len, s._len);
  }
}

SFC_TEST(index) {
  const auto s = Str{"abc"};
  panicking::assert_eq(s.len(), 3U);
  panicking::assert_true(s);
  panicking::assert_true(s.is_empty() == false);
  panicking::assert_eq(s[0], 'a');
  panicking::assert_eq(s[1], 'b');
  panicking::assert_eq(s[2], 'c');
  panicking::assert_eq(s[3], '\0');
}

SFC_TEST(cmp) {
  {
    const auto s = Str{"abc"};
    panicking::assert_eq(s, Str{"abc"});
    panicking::assert_ne(s, Str{"ab"});
    panicking::assert_ne(s, Str{"abcd"});
  }
}

SFC_TEST(range) {
  {
    const auto s = Str{"abc"};
    panicking::assert_eq(s[{0, 3}], Str{"abc"});
    panicking::assert_eq(s[{1, 3}], Str{"bc"});
    panicking::assert_eq(s[{0, 2}], Str{"ab"});
    panicking::assert_eq(s[{1, 2}], Str{"b"});
    panicking::assert_eq(s[{3, 3}], Str{""});
    panicking::assert_eq(s[{4, 5}], Str{""});

    panicking::assert_eq(s[{0, _}], Str{"abc"});
    panicking::assert_eq(s[{1, _}], Str{"bc"});
    panicking::assert_eq(s[{2, _}], Str{"c"});
    panicking::assert_eq(s[{3, _}], Str{""});
  }
}

SFC_TEST(search_next) {
  const auto s = Str{"aabbcc"};

  {
    auto x = s.search('a');
    panicking::assert_eq(x.next(), Option<usize>{0});
    panicking::assert_eq(x.next(), Option<usize>{1});
    panicking::assert_false(x.next());
  }

  {
    auto x = s.search("ab");
    panicking::assert_false(x.next());
    panicking::assert_eq(x.next(), Option<usize>{1});
    panicking::assert_false(x.next());
  }
}

SFC_TEST(search_next_back) {
  const auto s = Str{"aabbcc"};

  {
    auto x = s.search('b');
    panicking::assert_false(x.next_back());
    panicking::assert_false(x.next_back());
    panicking::assert_eq(x.next_back(), Option<usize>{3});
    panicking::assert_eq(x.next_back(), Option<usize>{2});
    panicking::assert_false(x.next_back());
  }

  {
    auto x = s.search("bc");
    panicking::assert_false(x.next_back());
    panicking::assert_eq(x.next_back(), Option<usize>{3});
    panicking::assert_false(x.next_back());
  }
}

SFC_TEST(search_match) {
  const auto s = Str{"aabbcc"};

  {
    auto x = s.search('c');
    panicking::assert_eq(x.next_match(), Option<usize>{4});
    panicking::assert_eq(x.next_match(), Option<usize>{5});
    panicking::assert_false(x.next_match());
  }

  {
    auto x = s.search("ab");
    panicking::assert_eq(x.next_match(), Option<usize>{1});
    panicking::assert_false(x.next_match());
  }
}

SFC_TEST(find) {
  const auto s = Str{"abcabc"};
  panicking::assert_eq(s.find('a'), Option<usize>{0});
  panicking::assert_eq(s.find('b'), Option<usize>{1});
  panicking::assert_eq(s.find('c'), Option<usize>{2});
  panicking::assert_false(s.find('d'));

  panicking::assert_eq(s.find("ab"), Option<usize>{0});
  panicking::assert_eq(s.find("bc"), Option<usize>{1});
  panicking::assert_eq(s.find("ac"), Option<usize>{});
}

SFC_TEST(rfind) {
  const auto s = Str{"abcabc"};
  panicking::assert_eq(s.rfind('a'), Option<usize>{3});
  panicking::assert_eq(s.rfind('b'), Option<usize>{4});
  panicking::assert_eq(s.rfind('c'), Option<usize>{5});
  panicking::assert_false(s.rfind('d'));

  panicking::assert_eq(s.rfind("ab"), Option<usize>{3});
  panicking::assert_eq(s.rfind("bc"), Option<usize>{4});
  panicking::assert_eq(s.rfind("ac"), Option<usize>{});
}

SFC_TEST(contains) {
  const auto s = Str{"abc"};
  panicking::assert_false(s.contains('0'));
  panicking::assert_true(s.contains('a'));
  panicking::assert_true(s.contains('b'));

  panicking::assert_true(s.contains("ab"));
  panicking::assert_true(s.contains("abc"));
  panicking::assert_false(s.contains("abcd"));
}

SFC_TEST(starts_with) {
  const auto s = Str{"abc"};
  panicking::assert_true(s.starts_with('a'));
  panicking::assert_false(s.starts_with('b'));

  panicking::assert_true(s.starts_with("ab"));
  panicking::assert_true(s.starts_with("abc"));
  panicking::assert_false(s.starts_with("abcd"));

  panicking::assert_true(s.starts_with([](auto c) { return c == 'a'; }));
  panicking::assert_false(s.starts_with([](auto c) { return c == 'b'; }));
}

SFC_TEST(ends_with) {
  const auto s = Str{"abc"};
  panicking::assert_true(s.ends_with('c'));
  panicking::assert_false(s.ends_with('b'));

  panicking::assert_true(s.ends_with("bc"));
  panicking::assert_true(s.ends_with("abc"));
  panicking::assert_false(s.ends_with("abcd"));

  panicking::assert_true(s.ends_with([](auto c) { return c == 'c'; }));
  panicking::assert_false(s.ends_with([](auto c) { return c == 'b'; }));
}

SFC_TEST(trim) {
  {
    const auto s = Str{"   abc   "};
    panicking::assert_eq(s.trim_start(), Str{"abc   "});
    panicking::assert_eq(s.trim_end(), Str{"   abc"});
    panicking::assert_eq(s.trim(), Str{"abc"});
  }

  {
    const auto s = Str{"abc"};
    panicking::assert_eq(s.trim_start(), Str{"abc"});
    panicking::assert_eq(s.trim_end(), Str{"abc"});
    panicking::assert_eq(s.trim(), Str{"abc"});
  }
}

SFC_TEST(trim_matches) {
  {
    const auto s = Str{"   abc   "};
    panicking::assert_eq(s.trim_start_matches(' '), Str{"abc   "});
    panicking::assert_eq(s.trim_end_matches(' '), Str{"   abc"});
    panicking::assert_eq(s.trim_matches(' '), Str{"abc"});
  }

  {
    const auto s = Str{"abc"};
    panicking::assert_eq(s.trim_start_matches(' '), Str{"abc"});
    panicking::assert_eq(s.trim_end_matches(' '), Str{"abc"});
    panicking::assert_eq(s.trim_matches(' '), Str{"abc"});
  }
}

}  // namespace sfc::str
