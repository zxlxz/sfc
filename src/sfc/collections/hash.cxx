#include "sfc/collections/hash.h"

#include "sfc/test.h"

namespace sfc::collections::test {

SFC_TEST(hashmap_get) {
  auto t = HashMap<int, int>{};
  t.insert(1, 10);
  auto v = t.get_mut(1);
  panicking::assert_true(v);

  *v = 11;
  panicking::assert_eq(t.get(1).unwrap(), 11);
}

SFC_TEST(hashmap_try_insert) {
  auto t = HashMap<int, int>{};

  for (auto i = 0; i < 5; ++i) {
    t.try_insert(i, i * 10);
    panicking::assert_eq(t.len(), i + 1);
  }

  panicking::assert_eq(*t.get(0), 0);
  panicking::assert_eq(*t.get(1), 10);
  panicking::assert_false(t.get(5));

  for (auto i = 5; i < 10; ++i) {
    t.try_insert(i, i * 10);
    panicking::assert_eq(t.len(), i + 1);
  }

  panicking::assert_eq(*t.get(5), 50);
  panicking::assert_eq(*t.get(9), 90);
  panicking::assert_false(t.get(10));

  for (auto i = 10; i < 100; ++i) {
    t.try_insert(i, i * 10);
    panicking::assert_eq(t.len(), i + 1);
    panicking::assert_eq(*t.get(i), i * 10);
  }

  for (auto i = 0; i < 100; ++i) {
    panicking::assert_eq(*t.get(i), i * 10);
  }

  panicking::assert_true(t.capacity() < t.len() * 4);
}

SFC_TEST(hashmap_insert) {
  auto t = HashMap<int, int>{};
  for (auto i = 0; i < 10; ++i) {
    t.insert(i, i * 10);
    panicking::assert_eq(*t.get(i), i * 10);
    t.insert(i, i * 100);
    panicking::assert_eq(*t.get(i), i * 100);
  }

  panicking::assert_eq(t.len(), 10U);
}

SFC_TEST(hashmap_remove) {
  auto t = HashMap<int, int>{};
  for (auto i = 0; i < 3; ++i) {
    t.insert(i, i * 10);
  }

  for (auto i = 0; i < 3; ++i) {
    panicking::assert_eq(t.remove(i), Option{i * 10});
    panicking::assert_false(t.get(i));
  }

  for (auto i = 0; i < 100; ++i) {
    t.insert(i, i * 10);
    panicking::assert_eq(*t.get(i), i * 10);
  }

  for (auto i = 0; i < 100; ++i) {
    panicking::assert_eq(t.remove(i), Option{i * 10});
  }

  panicking::assert_eq(t.len(), 0U);
}

}  // namespace sfc::collections::test
