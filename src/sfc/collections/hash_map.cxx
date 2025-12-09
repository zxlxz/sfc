#include "sfc/collections/hash_map.h"

#include "sfc/test/ut.h"

namespace sfc::collections::hash_map::test {

SFC_TEST(get) {
  auto t = HashMap<int, int>{};
  t.insert(1, 10);
  auto v = t.get_mut(1);
  panicking::expect(v);

  *v = 11;
  panicking::expect_eq(t.get(1).unwrap(), 11);
}

SFC_TEST(try_insert) {
  auto t = HashMap<int, int>{};

  for (auto i = 0; i < 5; ++i) {
    t.try_insert(i, i * 10);
    panicking::expect_eq(t.len(), i + 1);
  }

  panicking::expect_eq(*t.get(0), 0);
  panicking::expect_eq(*t.get(1), 10);
  panicking::expect(!t.get(5));

  for (auto i = 5; i < 10; ++i) {
    t.try_insert(i, i * 10);
    panicking::expect_eq(t.len(), i + 1);
  }

  panicking::expect_eq(*t.get(5), 50);
  panicking::expect_eq(*t.get(9), 90);
  panicking::expect(!t.get(10));

  for (auto i = 10; i < 100; ++i) {
    t.try_insert(i, i * 10);
    panicking::expect_eq(t.len(), i + 1);
    panicking::expect_eq(*t.get(i), i * 10);
  }

  for (auto i = 0; i < 100; ++i) {
    panicking::expect_eq(*t.get(i), i * 10);
  }

  panicking::expect(t.capacity() < t.len() * 4);
}

SFC_TEST(insert) {
  auto t = HashMap<int, int>{};
  for (auto i = 0; i < 10; ++i) {
    t.insert(i, i * 10);
    panicking::expect_eq(*t.get(i), i * 10);
    t.insert(i, i * 100);
    panicking::expect_eq(*t.get(i), i * 100);
  }

  panicking::expect_eq(t.len(), 10U);
}

SFC_TEST(remove) {
  auto t = HashMap<int, int>{};
  for (auto i = 0; i < 3; ++i) {
    t.insert(i, i * 10);
  }

  for (auto i = 0; i < 3; ++i) {
    panicking::expect_eq(t.remove(i), Option{i * 10});
    panicking::expect(!t.get(i));
  }

  for (auto i = 0; i < 100; ++i) {
    t.insert(i, i * 10);
    panicking::expect_eq(*t.get(i), i * 10);
  }

  for (auto i = 0; i < 100; ++i) {
    panicking::expect_eq(t.remove(i), Option{i * 10});
  }

  panicking::expect_eq(t.len(), 0U);
}

}  // namespace sfc::collections::test
