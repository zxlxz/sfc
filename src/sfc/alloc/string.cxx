#include "sfc/test.h"

namespace sfc::string::test {

SFC_TEST(property) {
  const auto test_str = String::from("abc");
  const auto null_str = String::from("");

  panicking::assert_eq(test_str.len(), 3U);
  panicking::assert_false(test_str.is_empty());

  panicking::assert_eq(null_str.len(), 0U);
  panicking::assert_true(null_str.is_empty());
}

SFC_TEST(index) {
  const auto s = String::from("abc");

  panicking::assert_eq(s[0], 'a');
  panicking::assert_eq(s[1], 'b');
  panicking::assert_eq(s[2], 'c');
}

SFC_TEST(index_range) {
  const auto s = String::from("abc");

  panicking::assert_eq((s[{0, _}]), "abc");
  panicking::assert_eq((s[{0, 2}]), "ab");
  panicking::assert_eq((s[{0, 8}]), "abc");

  panicking::assert_eq((s[{1, _}]), "bc");
  panicking::assert_eq((s[{1, 2}]), "b");

  panicking::assert_eq((s[{8, 9}]), "");
}

SFC_TEST(cmp) {
  const auto s = String::from("abcd");

  panicking::assert_true(s == "abcd");
  panicking::assert_false(s == "abc");
}

SFC_TEST(find) {
  const auto s = String::from("abc");

  panicking::assert_eq(s.find('a'), Option<usize>{0UL});
  panicking::assert_eq(s.find('b'), Option<usize>{1UL});
  panicking::assert_eq(s.rfind('c'), Option<usize>{2UL});
  panicking::assert_eq(s.find('d'), Option<usize>{});
}

SFC_TEST(rfind) {
  const auto s = String::from("abc");

  panicking::assert_eq(s.rfind('a'), Option<usize>{0UL});
  panicking::assert_eq(s.rfind('b'), Option<usize>{1UL});
  panicking::assert_eq(s.rfind('c'), Option<usize>{2UL});
  panicking::assert_eq(s.rfind('d'), Option<usize>{});
}

}  // namespace sfc::string
