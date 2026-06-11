#include "sfc/collections/hash/hash_map.h"
#include "sfc/test/test.h"

namespace sfc::collections::hash::test {

SFC_TEST(map_get) {
  auto t = HashMap<int, int>{};
  t.insert(1, 10);
  auto v = t.get_mut(1);
  sfc::assert_eq(v.is_some(), true);

  *v = 11;
  sfc::assert_eq(t.get(1), Option{11});
}

SFC_TEST(map_try_insert) {
  auto t = HashMap<u32, u32>{};

  for (auto i = 0U; i < 5U; ++i) {
    t.try_insert(i, i * 10);
    sfc::assert_eq(t.len(), i + 1);
  }

  sfc::assert_eq(t.get(0U), Option{0U});
  sfc::assert_eq(t.get(1U), Option{10U});
  sfc::assert_eq(t.get(5U), Option{});

  for (auto i = 5U; i < 10U; ++i) {
    t.try_insert(i, i * 10);
    sfc::assert_eq(t.len(), i + 1);
  }

  sfc::assert_eq(t.get(5U), Option{50U});
  sfc::assert_eq(t.get(9U), Option{90U});
  sfc::assert_eq(t.get(10U), Option{});
  for (auto i = 10U; i < 100U; ++i) {
    t.try_insert(i, i * 10);
    sfc::assert_eq(t.len(), i + 1);
    sfc::assert_eq(t.get(i), Option{i * 10});
  }

  for (auto i = 0U; i < 100U; ++i) {
    sfc::assert_eq(t.get(i), Option{i * 10});
  }

  sfc::assert_lt(t.capacity(), t.len() * 4);
}

SFC_TEST(map_insert) {
  auto t = HashMap<u32, u32>{};
  for (auto i = 0U; i < 10U; ++i) {
    t.insert(i, i * 10);
    sfc::assert_eq(t.get(i), Option{i * 10});
    sfc::assert_eq(t.len(), i + 1);
    t.insert(i, i * 100);
    sfc::assert_eq(t.get(i), Option{i * 100});
    sfc::assert_eq(t.len(), i + 1);
  }

  sfc::assert_eq(t.len(), 10U);
}

SFC_TEST(map_remove) {
  auto t = HashMap<int, int>{};
  for (auto i = 0; i < 3; ++i) {
    t.insert(i, i * 10);
  }

  for (auto i = 0; i < 3; ++i) {
    sfc::assert_eq(t.remove(i), Option{i * 10});
    sfc::assert_eq(t.get(i), Option{});
  }

  for (auto i = 0; i < 100; ++i) {
    t.insert(i, i * 10);
    sfc::assert_eq(t.get(i), Option{i * 10});
  }

  for (auto i = 0; i < 100; ++i) {
    sfc::assert_eq(t.remove(i), Option{i * 10});
  }

  sfc::assert_eq(t.len(), 0U);
}

}  // namespace sfc::collections::hash::test
