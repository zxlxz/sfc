#include "sfc/collections/hash.h"

#include "sfc/test.h"

namespace sfc::collections::test {

SFC_TEST(hashmap_get) {
  auto t = HashMap<int, int>{};
  t.insert(1, 10);
  auto v = t.get_mut(1);
  panicking::assert_true(v);
  if (v) {
    *v = 42;
  }
  panicking::assert_eq(t.get(1), Option{42});
}

SFC_TEST(hashmap_insert) {
  auto t = HashMap<int, int>{};

  for (auto i = 0; i < 5; ++i) {
    t.insert(i, i * 10);
    panicking::assert_eq(t.len(), i + 1);
  }

  panicking::assert_eq(t.get(0), Option{0});
  panicking::assert_eq(t.get(1), Option{10});
  panicking::assert_false(t.get(5));

  for (auto i = 5; i < 10; ++i) {
    t.insert(i, i * 10);
    panicking::assert_eq(t.len(), i + 1);
  }

  panicking::assert_eq(t.get(5), Option{50});
  panicking::assert_eq(t.get(9), Option{90});
  panicking::assert_false(t.get(10));

  for (auto i = 10; i < 100; ++i) {
    t.insert(i, i * 10);
    panicking::assert_eq(t.len(), i + 1);
    panicking::assert_eq(t.get(i), Option{i * 10});
  }

  for (auto i = 0; i < 100; ++i) {
    panicking::assert_eq(t.get(i), Option{i * 10});
  }

  panicking::assert_true(t.capacity() < t.len() * 2);
}

SFC_TEST(hashmap_replace) {
  auto t = HashMap<int, int>{};
  for (auto i = 0; i < 10; ++i) {
    t.replace(i, i * 10);
    panicking::assert_eq(t.get(i), Option{i * 10});
    t.replace(i, i * 100);
    panicking::assert_eq(t.get(i), Option{i * 100});
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
    panicking::assert_eq(t.get(i), Option{i * 10});
  }

  for (auto i = 0; i < 100; ++i) {
    panicking::assert_eq(t.remove(i), Option{i * 10});
  }

  panicking::assert_eq(t.len(), 0U);
}

}  // namespace sfc::collections::test
