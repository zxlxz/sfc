#include "sfc/test/test.h"

namespace sfc::vec::test {

SFC_TEST(index) {
  int tmp[] = {0, 1, 2, 3};

  auto v = Vec<int>::from(tmp);
  panicking::expect_eq(v[0], 0);
  panicking::expect_eq(v[1], 1);
}

SFC_TEST(slice) {
  int tmp[] = {0, 1, 2, 3};

  auto v = Vec<int>::from(tmp);
  auto s = v.as_slice();
  panicking::expect(s.as_ptr() == v.as_ptr());
  panicking::expect_eq(s.len(), v.len());
}

SFC_TEST(clone) {
  int tmp[] = {0, 1, 2, 3};

  auto x = Vec<int>::from(tmp);
  auto y = x.clone();
  for (auto i = 0U; i < x.len(); ++i) {
    panicking::expect_eq(x[i], i);
    panicking::expect_eq(y[i], i);
  }
}

SFC_TEST(push) {
  int tmp[] = {0, 1, 2, 3};

  auto v = Vec<int>::from(tmp);
  v.push(4);
  v.push(5);
  panicking::expect_eq(v.len(), 6U);
  panicking::expect_eq(v.last(), Option{5});
}

SFC_TEST(pop) {
  int tmp[] = {0, 1, 2, 3};

  auto v = Vec<int>::from(tmp);
  panicking::expect_eq(v.pop().unwrap(), 3);
  panicking::expect_eq(v.pop().unwrap(), 2);
  panicking::expect_eq(v.pop().unwrap(), 1);
  panicking::expect_eq(v.pop().unwrap(), 0);
};

SFC_TEST(insert) {
  int tmp[] = {0, 1, 3};

  auto v = Vec<int>::from(tmp);
  v.insert(2U, 2);
  v.insert(4U, 4);
  panicking::expect_eq(v.len(), 5U);
  for (auto i = 0U; i < 5U; ++i) {
    panicking::expect_eq(v[i], i);
  }
}

SFC_TEST(remove) {
  int tmp[] = {0, 1, 2, 3};

  auto v = Vec<int>::from(tmp);
  v.remove(1);
  panicking::expect_eq(v.len(), 3U);
  panicking::expect_eq(v[2], 3);
}

SFC_TEST(reserve) {
  auto v = Vec<int>::with_capacity(10);
  panicking::expect_eq(v.capacity(), 10U);
  for (auto i = 0; i < 10; ++i) {
    v.push(i);
  }
  v.reserve(1);
  panicking::expect(v.capacity() >= 16);
}

SFC_TEST(drain) {
  int tmp[] = {0, 1, 2, 3, 4, 5};

  auto v = Vec<int>::from(tmp);
  v.drain(ops::Range{2, 4});
  panicking::expect_eq(v.len(), 4U);
  panicking::expect_eq(v[0], 0);
  panicking::expect_eq(v[1], 1);
  panicking::expect_eq(v[2], 4);
  panicking::expect_eq(v[3], 5);
}
}  // namespace sfc::vec::test
