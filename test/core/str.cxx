#include "sfc/test.h"

namespace sfc::str {

SFC_TEST(property) {
  const auto test_str = Str{"abc"};
  const auto null_str = Str{""};

  test::assert_eq(test_str.len(), 3U);
  test::assert_false(test_str.is_empty());

  test::assert_eq(null_str.len(), 0U);
  test::assert_true(null_str.is_empty());
}

SFC_TEST(index) {
  auto s = Str{"abc"};

  test::assert_eq(s[0], 'a');
  test::assert_eq(s[1], 'b');
  test::assert_eq(s[2], 'c');
  test::assert_eq(s[3], char(0));
}

SFC_TEST(index_range) {
  auto s = Str{"abc"};

  test::assert_eq((s[{0, _}]), "abc");
  test::assert_eq((s[{0, 2}]), "ab");
  test::assert_eq((s[{0, 8}]), "abc");

  test::assert_eq((s[{1, _}]), "bc");
  test::assert_eq((s[{1, 2}]), "b");

  test::assert_eq((s[{8, 9}]), "");
}

SFC_TEST(cmp) {
  const auto s = Str{"abcd"};

  test::assert_true(s == "abcd");
  test::assert_false(s == "abc");
}

SFC_TEST(iter) {
  const auto s = Str{"abc"};

  auto n = 0;
  for (auto c : s.as_chars()) {
    test::assert_eq(c, 'a' + n);
    n += 1;
  }
}

SFC_TEST(find) {
  const auto s = Str{"abc"};

  test::assert_eq(s.find('a'), 0UL);
  test::assert_eq(s.find('b'), 1UL);
  test::assert_eq(s.rfind('c'), 2UL);
  test::assert_eq(s.find('d'), Option<usize>{});
}

SFC_TEST(rfind) {
  const auto s = Str{"abc"};

  test::assert_eq(s.rfind('a'), 0UL);
  test::assert_eq(s.rfind('b'), 1UL);
  test::assert_eq(s.rfind('c'), 2UL);
  test::assert_eq(s.rfind('d'), Option<usize>{});
}

}  // namespace sfc::str
