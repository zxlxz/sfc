#include "sfc/test.h"

namespace sfc::vec {

SFC_TEST(index) {
  auto v = Vec<int>::from((int[]){0, 1, 2, 3});
  test::assert_eq(v[0], 0);
  test::assert_eq(v[1], 1);
}

SFC_TEST(slice) {
  auto v = Vec<int>::from((int[]){0, 1, 2, 3});
  auto s = v.as_slice();
  test::assert_true(s.as_ptr() == v.as_ptr());
  test::assert_eq(s.len(), v.len());
}

SFC_TEST(clone) {
  auto x = Vec<u32>::from((u32[]){0U, 1U, 2U, 3U});
  auto y = x.clone();
  for (auto i = 0U; i < x.len(); ++i) {
    test::assert_eq(x[i], i);
    test::assert_eq(y[i], i);
  }
}

SFC_TEST(push) {
  auto v = Vec<int>::from((int[]){0, 1, 2, 3});
  v.push(4);
  v.push(5);
  test::assert_eq(v.len(), 6U);
  test::assert_eq(v.last(), 5);
}

SFC_TEST(pop) {
  auto v = Vec<int>::from((int[]){0, 1, 2, 3});
  test::assert_eq(*v.pop(), 3);
  test::assert_eq(*v.pop(), 2);
  test::assert_eq(*v.pop(), 1);
  test::assert_eq(*v.pop(), 0);
};

SFC_TEST(insert) {
  auto v = Vec<int>::from((int[]){0, 2, 3});
  v.insert(1, 1);
  v.insert(8, 4);
  test::assert_eq(v.len(), 5U);
  test::assert_eq(v[1], 1);
  test::assert_eq(v[4], 4);
}

SFC_TEST(remove) {
  auto v = Vec<int>::from((int[]){0, 1, 1, 2});
  v.remove(1);
  test::assert_eq(v.len(), 3U);
  test::assert_eq(v[2], 2);
}

SFC_TEST(retain) {}

}  // namespace sfc::vec
