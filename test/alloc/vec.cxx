#include "sfc/test.h"

namespace sfc::vec {

SFC_TEST(index) {
  auto v = Vec<int>::from((int[]){0, 1, 2, 3});
  panicking::assert_eq(v[0], 0);
  panicking::assert_eq(v[1], 1);
}

SFC_TEST(slice) {
  auto v = Vec<int>::from((int[]){0, 1, 2, 3});
  auto s = v.as_slice();
  panicking::assert_true(s.as_ptr() == v.as_ptr());
  panicking::assert_eq(s.len(), v.len());
}

SFC_TEST(clone) {
  auto x = Vec<u32>::from((u32[]){0U, 1U, 2U, 3U});
  auto y = x.clone();
  for (auto i = 0U; i < x.len(); ++i) {
    panicking::assert_eq(x[i], i);
    panicking::assert_eq(y[i], i);
  }
}

SFC_TEST(push) {
  auto v = Vec<int>::from((int[]){0, 1, 2, 3});
  v.push(4);
  v.push(5);
  panicking::assert_eq(v.len(), 6U);
  panicking::assert_eq(v.last(), 5);
}

SFC_TEST(pop) {
  auto v = Vec<int>::from((int[]){0, 1, 2, 3});
  panicking::assert_eq(*v.pop(), 3);
  panicking::assert_eq(*v.pop(), 2);
  panicking::assert_eq(*v.pop(), 1);
  panicking::assert_eq(*v.pop(), 0);
};

SFC_TEST(insert) {
  auto v = Vec<int>::from((int[]){0, 2, 3});
  v.insert(1U, 1);
  v.insert(4U, 4);
  panicking::assert_eq(v.len(), 5U);
  panicking::assert_eq(v[1], 1);
  panicking::assert_eq(v[4], 4);
}

SFC_TEST(remove) {
  auto v = Vec<int>::from((int[]){0, 1, 2, 3});
  v.remove(1);
  panicking::assert_eq(v.len(), 3U);
  panicking::assert_eq(v[2], 3);
}

SFC_TEST(retain) {}

}  // namespace sfc::vec
