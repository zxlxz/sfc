#include "sfc/collections/hash_map.h"

#include "sfc/test/test.h"

namespace sfc::collections::hash_map::test {

SFC_TEST(get) {
  auto t = HashMap<int, int>{};
  t.insert(1, 10);
  auto v = t.get_mut(1);
  sfc::expect_true(v);

  *v = 11;
  sfc::expect_eq(t.get(1).unwrap(), 11);
}

SFC_TEST(try_insert) {
  auto t = HashMap<int, int>{};

  for (auto i = 0; i < 5; ++i) {
    t.try_insert(i, i * 10);
    sfc::expect_eq(t.len(), i + 1);
  }

  sfc::expect_eq(*t.get(0), 0);
  sfc::expect_eq(*t.get(1), 10);
  sfc::expect_false(t.get(5));

  for (auto i = 5; i < 10; ++i) {
    t.try_insert(i, i * 10);
    sfc::expect_eq(t.len(), i + 1);
  }

  sfc::expect_eq(*t.get(5), 50);
  sfc::expect_eq(*t.get(9), 90);
  sfc::expect_false(t.get(10));

  for (auto i = 10; i < 100; ++i) {
    t.try_insert(i, i * 10);
    sfc::expect_eq(t.len(), i + 1);
    sfc::expect_eq(*t.get(i), i * 10);
  }

  for (auto i = 0; i < 100; ++i) {
    sfc::expect_eq(*t.get(i), i * 10);
  }

  sfc::expect_true(t.capacity() < t.len() * 4);
}

SFC_TEST(insert) {
  auto t = HashMap<int, int>{};
  for (auto i = 0; i < 10; ++i) {
    t.insert(i, i * 10);
    sfc::expect_eq(*t.get(i), i * 10);
    t.insert(i, i * 100);
    sfc::expect_eq(*t.get(i), i * 100);
  }

  sfc::expect_eq(t.len(), 10U);
}

SFC_TEST(remove) {
  auto t = HashMap<int, int>{};
  for (auto i = 0; i < 3; ++i) {
    t.insert(i, i * 10);
  }

  for (auto i = 0; i < 3; ++i) {
    sfc::expect_eq(t.remove(i), Option{i * 10});
    sfc::expect_false(t.get(i));
  }

  for (auto i = 0; i < 100; ++i) {
    t.insert(i, i * 10);
    sfc::expect_eq(*t.get(i), i * 10);
  }

  for (auto i = 0; i < 100; ++i) {
    sfc::expect_eq(t.remove(i), Option{i * 10});
  }

  sfc::expect_eq(t.len(), 0U);
}

}  // namespace sfc::collections::test
