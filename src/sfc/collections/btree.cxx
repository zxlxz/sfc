#include <stdio.h>
#include "sfc/collections/btree.h"

#include "sfc/test.h"

namespace sfc::collections::test {

SFC_TEST(btree_get) {
  auto t = BTreeMap<int, int>{};
  t.insert(1, 10);
  auto v = t.get_mut(1);
  panicking::expect_true(v);
  if (v) {
    *v = 42;
  }
  panicking::expect_eq(t.get(1), Option{42});
}

SFC_TEST(btree_try_insert) {
  auto t = BTreeMap<int, int>{};

  for (auto i = 0; i < 5; ++i) {
    t.try_insert(i, i * 10);
    panicking::expect_eq(t.len(), i + 1);
  }

  panicking::expect_eq(t.get(0), Option{0});
  panicking::expect_eq(t.get(1), Option{10});
  panicking::expect_false(t.get(5));

  for (auto i = 5; i < 10; ++i) {
    t.try_insert(i, i * 10);
    panicking::expect_eq(t.len(), i + 1);
  }

  panicking::expect_eq(t.get(5), Option{50});
  panicking::expect_eq(t.get(9), Option{90});
  panicking::expect_false(t.get(10));

  for (auto i = 10; i < 100; ++i) {
    t.try_insert(i, i * 10);
    panicking::expect_eq(t.len(), i + 1);
    panicking::expect_eq(t.get(i), Option{i * 10});
  }

  for (auto i = 0; i < 100; ++i) {
    panicking::expect_eq(t.get(i), Option{i * 10});
  }
}

SFC_TEST(btree_insert) {
  auto t = BTreeMap<int, int>{};
  for (auto i = 0; i < 10; ++i) {
    t.insert(i, i * 10);
    panicking::expect_eq(t.get(i), Option{i * 10});
    t.insert(i, i * 100);
    panicking::expect_eq(t.get(i), Option{i * 100});
  }

  panicking::expect_eq(t.len(), 10U);
}

SFC_TEST(btree_remove) {
  auto t = BTreeMap<int, int>{};
  for (auto i = 0; i < 3; ++i) {
    t.insert(i, i * 10);
  }

  for (auto i = 0; i < 3; ++i) {
    panicking::expect_eq(t.remove(i), Option{i * 10});
    panicking::expect_false(t.get(i));
  }

  for (auto i = 0; i < 100; ++i) {
    t.insert(i, i * 10);
    panicking::expect_eq(t.get(i), Option{i * 10});
  }

  for (auto i = 0; i < 10; ++i) {
    panicking::expect_eq(t.remove(i), Option{i * 10});
  }

  for (auto i = 10; i < 23; ++i) {
    panicking::expect_eq(t.remove(i), Option{i * 10});
  }

  for (auto i = 23; i < 100; ++i) {
    panicking::expect_eq(t.remove(i), Option{i * 10});
  }

  panicking::expect_eq(t.len(), 0U);
}

}  // namespace sfc::collections::test
