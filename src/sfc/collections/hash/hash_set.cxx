#include "sfc/collections/hash.h"
#include "sfc/test/test.h"

namespace sfc::collections::hash::test {

SFC_TEST(set_contains) {
  auto t = HashSet<int>{};
  sfc::assert_eq(t.contains(1), false);

  t.insert(1);
  sfc::assert_eq(t.contains(1), true);
  sfc::assert_eq(t.contains(2), false);

  t.insert(2);
  t.insert(3);
  sfc::assert_eq(t.contains(1), true);
  sfc::assert_eq(t.contains(2), true);
  sfc::assert_eq(t.contains(3), true);
  sfc::assert_eq(t.contains(4), false);
}

SFC_TEST(set_insert) {
  auto t = HashSet<int>{};

  // First insert should return true (newly inserted)
  sfc::assert_eq(t.insert(1), true);
  sfc::assert_eq(t.len(), 1U);
  sfc::assert_eq(t.contains(1), true);

  // Second insert of same value should return false (already exists)
  sfc::assert_eq(t.insert(1), false);
  sfc::assert_eq(t.len(), 1U);  // Length unchanged

  // Insert new values
  sfc::assert_eq(t.insert(2), true);
  sfc::assert_eq(t.insert(3), true);
  sfc::assert_eq(t.len(), 3U);

  // Verify all values present
  sfc::assert_eq(t.contains(1), true);
  sfc::assert_eq(t.contains(2), true);
  sfc::assert_eq(t.contains(3), true);
}

SFC_TEST(set_remove) {
  auto t = HashSet<int>{};

  // Remove from empty set
  sfc::assert_eq(t.remove(1), false);

  // Add some values
  for (auto i = 0; i < 3; ++i) {
    t.insert(i);
  }
  sfc::assert_eq(t.len(), 3U);

  // Remove existing values
  sfc::assert_eq(t.remove(0), true);
  sfc::assert_eq(t.contains(0), false);
  sfc::assert_eq(t.len(), 2U);

  sfc::assert_eq(t.remove(1), true);
  sfc::assert_eq(t.contains(1), false);
  sfc::assert_eq(t.len(), 1U);

  sfc::assert_eq(t.remove(2), true);
  sfc::assert_eq(t.contains(2), false);
  sfc::assert_eq(t.len(), 0U);

  // Remove already removed value
  sfc::assert_eq(t.remove(2), false);
}

SFC_TEST(set_clear) {
  auto t = HashSet<int>{};

  for (auto i = 0; i < 100; ++i) {
    t.insert(i);
  }
  sfc::assert_eq(t.len(), 100U);

  t.clear();
  sfc::assert_eq(t.len(), 0U);

  // Verify all values gone
  for (auto i = 0; i < 100; ++i) {
    sfc::assert_eq(t.contains(i), false);
  }

  // Can insert again after clear
  t.insert(42);
  sfc::assert_eq(t.contains(42), true);
  sfc::assert_eq(t.len(), 1U);
}

SFC_TEST(set_with_capacity) {
  auto t = HashSet<int>::with_capacity(50);
  sfc::assert_eq(t.len(), 0U);
  sfc::assert_ge(t.capacity(), 50U);

  for (auto i = 0; i < 100; ++i) {
    t.insert(i);
  }
  sfc::assert_eq(t.len(), 100U);
}

SFC_TEST(set_reserve) {
  auto t = HashSet<int>{};
  sfc::assert_eq(t.capacity(), 0U);

  t.reserve(100);
  sfc::assert_ge(t.capacity(), 100U);

  // Should not reallocate when inserting within capacity
  auto cap_before = t.capacity();
  for (auto i = 0; i < 50; ++i) {
    t.insert(i);
  }
  sfc::assert_eq(t.capacity(), cap_before);
}

SFC_TEST(set_duplicate_inserts) {
  auto t = HashSet<int>{};

  // Insert same value multiple times
  for (auto i = 0; i < 10; ++i) {
    auto result = t.insert(42);
    if (i == 0) {
      sfc::assert_eq(result, true);  // First insert
    } else {
      sfc::assert_eq(result, false);  // Subsequent inserts
    }
  }

  sfc::assert_eq(t.len(), 1U);
  sfc::assert_eq(t.contains(42), true);
}

}  // namespace sfc::collections::hash::test
