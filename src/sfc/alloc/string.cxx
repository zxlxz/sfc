#include "sfc/test/test.h"

namespace sfc::string::test {

SFC_TEST(property) {
  const auto test_str = String::from("abc");
  const auto null_str = String::from("");

  sfc::assert_eq(test_str.len(), 3U);
  sfc::assert_eq(test_str.is_empty(), false);

  sfc::assert_eq(null_str.len(), 0U);
  sfc::assert_eq(null_str.is_empty(), true);
}

SFC_TEST(index) {
  const auto s = String::from("abc");

  sfc::assert_eq((s[{0, $}]), "abc");
  sfc::assert_eq((s[{0, 2}]), "ab");
  sfc::assert_eq((s[{0, 8}]), "abc");

  sfc::assert_eq((s[{1, $}]), "bc");
  sfc::assert_eq((s[{1, 2}]), "b");

  sfc::assert_eq((s[{8, 9}]), "");
}

SFC_TEST(cmp) {
  const auto s = String::from("abcd");
  sfc::assert_eq(s, "abcd");
  sfc::assert_ne(s, "abc");
}

SFC_TEST(find) {
  const auto s = String::from("abc");

  sfc::assert_eq(s.find('a'), Option<usize>{0UL});
  sfc::assert_eq(s.find('b'), Option<usize>{1UL});
  sfc::assert_eq(s.rfind('c'), Option<usize>{2UL});
  sfc::assert_eq(s.find('d'), Option<usize>{});
}

SFC_TEST(rfind) {
  const auto s = String::from("abc");

  sfc::assert_eq(s.rfind('a'), Option<usize>{0UL});
  sfc::assert_eq(s.rfind('b'), Option<usize>{1UL});
  sfc::assert_eq(s.rfind('c'), Option<usize>{2UL});
  sfc::assert_eq(s.rfind('d'), Option<usize>{});
}

}  // namespace sfc::string::test
