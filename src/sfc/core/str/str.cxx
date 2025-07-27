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
    panicking::assert_eq(s.len(), 3U);
    panicking::assert_true(s);
    panicking::assert_true(s.is_empty() == false);
    panicking::assert_eq(s[0], 'a');
    panicking::assert_eq(s[1], 'b');
    panicking::assert_eq(s[2], 'c');
    panicking::assert_eq(s[3], '\0');
  }

  {
    const auto s = Str{"abc"};
    panicking::assert_eq(s.as_ptr(), s._ptr);
    panicking::assert_eq(s.as_chars()._len, s._len);
    panicking::assert_eq(s.as_chars()._ptr, s._ptr);
    panicking::assert_eq(s.as_bytes()._len, s._len);
  }
}

SFC_TEST(cmp) {
  {
    const auto s = Str{"abc"};
    panicking::assert_eq(s, Str{"abc"});
    panicking::assert_ne(s, Str{"ab"});
    panicking::assert_ne(s, Str{"abcd"});
  }
}

SFC_TEST(ops) {
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

SFC_TEST(search) {
  // find
  {
    const auto s = Str{"abcabc"};
    panicking::assert_eq(s.find('a'), Option<usize>{0});
    panicking::assert_eq(s.find('b'), Option<usize>{1});
    panicking::assert_eq(s.find('c'), Option<usize>{2});
    panicking::assert_false(s.find('d'));
  }

  // rfind
  {
    const auto s = Str{"abcabc"};
    panicking::assert_eq(s.rfind('a'), Option<usize>{3});
    panicking::assert_eq(s.rfind('b'), Option<usize>{4});
    panicking::assert_eq(s.rfind('c'), Option<usize>{5});
    panicking::assert_false(s.rfind('d'));
  }

  // contains
  {
    const auto s = Str{"abc"};
    panicking::assert_eq(s.contains('a'), true);
    panicking::assert_eq(s.contains('b'), true);
    panicking::assert_eq(s.contains('c'), true);
    panicking::assert_eq(s.contains('d'), false);
  }

  // starts_with
  {
    const auto s = Str{"abc"};
    panicking::assert_true(s.starts_with('a'));
    panicking::assert_false(s.starts_with('b'));

    panicking::assert_true(s.starts_with("ab"));
    panicking::assert_true(s.starts_with("abc"));
    panicking::assert_false(s.starts_with("abcd"));

    panicking::assert_true(s.starts_with([](auto c) { return c == 'a'; }));
    panicking::assert_false(s.starts_with([](auto c) { return c == 'b'; }));
  }

  // ends_with
  {
    const auto s = Str{"abc"};
    panicking::assert_true(s.ends_with('c'));
    panicking::assert_false(s.ends_with('b'));

    panicking::assert_true(s.ends_with("bc"));
    panicking::assert_true(s.ends_with("abc"));
    panicking::assert_false(s.ends_with("abcd"));

    panicking::assert_true(s.ends_with([](auto c) { return c == 'c'; }));
    panicking::assert_false(s.ends_with([](auto c) { return c == 'b'; }));
  }
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