#include "sfc/test/test.h"

namespace sfc::string::test {

SFC_TEST(property) {
  const auto test_str = String::from("abc");
  const auto null_str = String::from("");

  sfc::expect_eq(test_str.len(), 3U);
  sfc::expect_true(!test_str.is_empty());

  sfc::expect_eq(null_str.len(), 0U);
  sfc::expect_true(null_str.is_empty());
}

SFC_TEST(index) {
  const auto s = String::from("abc");

  sfc::expect_eq(s[0], 'a');
  sfc::expect_eq(s[1], 'b');
  sfc::expect_eq(s[2], 'c');
}

SFC_TEST(index_range) {
  const auto s = String::from("abc");

  sfc::expect_eq((s[{0, $}]), "abc");
  sfc::expect_eq((s[{0, 2}]), "ab");
  sfc::expect_eq((s[{0, 8}]), "abc");

  sfc::expect_eq((s[{1, $}]), "bc");
  sfc::expect_eq((s[{1, 2}]), "b");

  sfc::expect_eq((s[{8, 9}]), "");
}

SFC_TEST(cmp) {
  const auto s = String::from("abcd");
  sfc::expect_eq(s, "abcd");
  sfc::expect_ne(s, "abc");
}

SFC_TEST(find) {
  const auto s = String::from("abc");

  sfc::expect_eq(s.find('a'), Option<usize>{0UL});
  sfc::expect_eq(s.find('b'), Option<usize>{1UL});
  sfc::expect_eq(s.rfind('c'), Option<usize>{2UL});
  sfc::expect_eq(s.find('d'), Option<usize>{});
}

SFC_TEST(rfind) {
  const auto s = String::from("abc");

  sfc::expect_eq(s.rfind('a'), Option<usize>{0UL});
  sfc::expect_eq(s.rfind('b'), Option<usize>{1UL});
  sfc::expect_eq(s.rfind('c'), Option<usize>{2UL});
  sfc::expect_eq(s.rfind('d'), Option<usize>{});
}

}  // namespace sfc::string::test
