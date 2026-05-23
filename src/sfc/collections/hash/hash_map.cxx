#include "sfc/collections/hash/hash_map.h"
#include "sfc/test/test.h"

namespace sfc::collections::hash::test {

SFC_TEST(map_get) {
  auto t = HashMap<int, int>{};
  t.insert(1, 10);
  auto v = t.get_mut(1);
  sfc::expect_true(v);

  *v = 11;
  sfc::expect_eq(t.get(1).unwrap(), 11);
}

SFC_TEST(map_try_insert) {
  auto t = HashMap<u32, u32>{};

  for (auto i = 0U; i < 5U; ++i) {
    t.try_insert(i, i * 10);
    sfc::expect_eq(t.len(), i + 1);
  }

  sfc::expect_eq(*t.get(0U), 0U);
  sfc::expect_eq(*t.get(1U), 10U);
  sfc::expect_false(t.get(5U));

  for (auto i = 5U; i < 10U; ++i) {
    t.try_insert(i, i * 10);
    sfc::expect_eq(t.len(), i + 1);
  }

  sfc::expect_eq(*t.get(5U), 50U);
  sfc::expect_eq(*t.get(9U), 90U);
  sfc::expect_false(t.get(10U));

  for (auto i = 10U; i < 100U; ++i) {
    t.try_insert(i, i * 10);
    sfc::expect_eq(t.len(), i + 1);
    sfc::expect_eq(*t.get(i), i * 10);
  }

  for (auto i = 0U; i < 100U; ++i) {
    sfc::expect_eq(*t.get(i), i * 10);
  }

  sfc::expect_true(t.capacity() < t.len() * 4);
}

SFC_TEST(map_insert) {
  auto t = HashMap<u32, u32>{};
  for (auto i = 0U; i < 10U; ++i) {
    t.insert(i, i * 10);
    sfc::expect_eq(*t.get(i), i * 10);
    sfc::expect_eq(t.len(), i + 1);
    t.insert(i, i * 100);
    sfc::expect_eq(*t.get(i), i * 100);
    sfc::expect_eq(t.len(), i + 1);
  }

  sfc::expect_eq(t.len(), 10U);
}

SFC_TEST(map_remove) {
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

}  // namespace sfc::collections::hash::test
