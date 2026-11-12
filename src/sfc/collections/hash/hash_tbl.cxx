#include "sfc/collections/hash/hash_tbl.h"
#include "sfc/test/test.h"

namespace sfc::collections::hash::test {

struct Entry {
  u32 key;
  u32 val;
};

SFC_TEST(hash_tbl_insert_replace) {
  auto t = HashTbl<Entry>{};
  sfc::assert_eq(t.insert(Entry{1, 10}).is_some(), false);

  sfc::assert_eq(t.insert(Entry{1, 11}).unwrap().val, 10U);
  sfc::assert_eq(t.len(), 1U);
  sfc::assert_eq(t.search(1U)->val, 11U);
}

SFC_TEST(hash_tbl_try_insert) {
  auto t = HashTbl<Entry>{};
  sfc::assert_eq(t.try_insert(Entry{2, 20}).is_some(), false);

  // existing key: returns the stored entry and keeps the old value
  sfc::assert_eq(t.try_insert(Entry{2, 21}).unwrap().val, 20U);
  sfc::assert_eq(t.len(), 1U);
  sfc::assert_eq(t.search(2U)->val, 20U);
}

SFC_TEST(hash_tbl_grow) {
  constexpr auto n = 1000U;
  auto t = HashTbl<Entry>{};
  for (auto i = 0U; i < n; ++i) {
    t.insert(Entry{i, i * 2});
  }

  sfc::assert_eq(t.len(), usize{n});
  sfc::assert_ge(t.cap(), usize{n});
  for (auto i = 0U; i < n; ++i) {
    sfc::assert_eq(t.search(i)->val, i * 2);
  }
  sfc::assert_eq(t.search(n) == nullptr, true);
}

SFC_TEST(hash_tbl_remove_reinsert) {
  auto t = HashTbl<Entry>{};
  for (auto i = 0U; i < 64U; ++i) {
    t.insert(Entry{i, i});
  }

  for (auto i = 0U; i < 64U; i += 2) {
    sfc::assert_eq(t.remove(i).unwrap().val, i);
  }
  sfc::assert_eq(t.remove(0U).is_some(), false);
  sfc::assert_eq(t.len(), 32U);

  // odd keys stay reachable past tombstones, and iteration skips them
  for (auto i = 1U; i < 64U; i += 2) {
    sfc::assert_eq(t.search(i)->val, i);
  }
  for (auto i = 0U; i < 64U; i += 2) {
    sfc::assert_eq(t.search(i) == nullptr, true);
  }
  auto cnt = usize{0};
  t.iter().for_each([&](const Entry&) { cnt += 1; });
  sfc::assert_eq(cnt, 32U);

  // reinsert through tombstones
  for (auto i = 0U; i < 64U; i += 2) {
    t.insert(Entry{i, i + 100});
  }
  sfc::assert_eq(t.len(), 64U);
  for (auto i = 0U; i < 64U; i += 2) {
    sfc::assert_eq(t.search(i)->val, i + 100);
  }
}

SFC_TEST(hash_tbl_churn) {
  // distinct keys inserted and removed repeatedly must not lose entries
  auto t = HashTbl<Entry>{};
  for (auto i = 0U; i < 10000U; ++i) {
    t.insert(Entry{i, i});
    t.remove(i);
  }
  sfc::assert_eq(t.len(), 0U);

  for (auto i = 0U; i < 16U; ++i) {
    t.insert(Entry{i, i});
  }
  sfc::assert_eq(t.len(), 16U);
  for (auto i = 0U; i < 16U; ++i) {
    sfc::assert_eq(t.search(i)->val, i);
  }
}

SFC_TEST(hash_tbl_move) {
  auto a = HashTbl<Entry>{};
  for (auto i = 0U; i < 100U; ++i) {
    a.insert(Entry{i, i});
  }

  auto b = mem::move(a);
  sfc::assert_eq(a.len(), 0U);
  sfc::assert_eq(b.len(), 100U);
  sfc::assert_eq(b.search(42U)->val, 42U);

  auto c = HashTbl<Entry>{};
  c.insert(Entry{7, 7});
  c = mem::move(b);
  sfc::assert_eq(c.len(), 100U);
  sfc::assert_eq(c.search(99U)->val, 99U);
  sfc::assert_eq(c.search(7U)->val, 7U);
}

SFC_TEST(hash_tbl_clear) {
  auto t = HashTbl<Entry>{};
  for (auto i = 0U; i < 50U; ++i) {
    t.insert(Entry{i, i});
  }

  t.clear();
  sfc::assert_eq(t.len(), 0U);
  sfc::assert_eq(t.search(10U) == nullptr, true);

  for (auto i = 0U; i < 50U; ++i) {
    t.insert(Entry{i, i + 1});
  }
  sfc::assert_eq(t.len(), 50U);
  sfc::assert_eq(t.search(10U)->val, 11U);
}

SFC_TEST(hash_tbl_with_capacity) {
  sfc::assert_eq(HashTbl<Entry>::with_capacity(0).cap(), 0U);

  auto t = HashTbl<Entry>::with_capacity(100);
  sfc::assert_eq(t.cap(), 128U);
  sfc::assert_eq(t.len(), 0U);
}

}  // namespace sfc::collections::hash::test
