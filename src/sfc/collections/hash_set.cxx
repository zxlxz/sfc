#include "sfc/collections/hash_set.h"

#include "sfc/test/test.h"

namespace sfc::collections::hash_set::test {

SFC_TEST(contains) {
  auto t = HashSet<int>{};
  panicking::expect_false(t.contains(1));

  t.insert(1);
  panicking::expect_true(t.contains(1));
  panicking::expect_false(t.contains(2));

  t.insert(2);
  t.insert(3);
  panicking::expect_true(t.contains(1));
  panicking::expect_true(t.contains(2));
  panicking::expect_true(t.contains(3));
  panicking::expect_false(t.contains(4));
}

SFC_TEST(insert) {
  auto t = HashSet<int>{};

  // First insert should return true (newly inserted)
  panicking::expect_true(t.insert(1));
  panicking::expect_eq(t.len(), 1U);
  panicking::expect_true(t.contains(1));

  // Second insert of same value should return false (already exists)
  panicking::expect_false(t.insert(1));
  panicking::expect_eq(t.len(), 1U);  // Length unchanged

  // Insert new values
  panicking::expect_true(t.insert(2));
  panicking::expect_true(t.insert(3));
  panicking::expect_eq(t.len(), 3U);

  // Verify all values present
  panicking::expect_true(t.contains(1));
  panicking::expect_true(t.contains(2));
  panicking::expect_true(t.contains(3));
}

SFC_TEST(remove) {
  auto t = HashSet<int>{};

  // Remove from empty set
  panicking::expect_false(t.remove(1));

  // Add some values
  for (auto i = 0; i < 3; ++i) {
    t.insert(i);
  }
  panicking::expect_eq(t.len(), 3U);

  // Remove existing values
  panicking::expect_true(t.remove(0));
  panicking::expect_false(t.contains(0));
  panicking::expect_eq(t.len(), 2U);

  panicking::expect_true(t.remove(1));
  panicking::expect_false(t.contains(1));
  panicking::expect_eq(t.len(), 1U);

  panicking::expect_true(t.remove(2));
  panicking::expect_false(t.contains(2));
  panicking::expect_eq(t.len(), 0U);

  // Remove already removed value
  panicking::expect_false(t.remove(2));
}

SFC_TEST(clear) {
  auto t = HashSet<int>{};

  for (auto i = 0; i < 100; ++i) {
    t.insert(i);
  }
  panicking::expect_eq(t.len(), 100U);

  t.clear();
  panicking::expect_eq(t.len(), 0U);

  // Verify all values gone
  for (auto i = 0; i < 100; ++i) {
    panicking::expect_false(t.contains(i));
  }

  // Can insert again after clear
  t.insert(42);
  panicking::expect_true(t.contains(42));
  panicking::expect_eq(t.len(), 1U);
}

SFC_TEST(with_capacity) {
  auto t = HashSet<int>::with_capacity(50);
  panicking::expect_eq(t.len(), 0U);
  panicking::expect_true(t.capacity() >= 50);

  for (auto i = 0; i < 100; ++i) {
    t.insert(i);
  }
  panicking::expect_eq(t.len(), 100U);
}

SFC_TEST(reserve) {
  auto t = HashSet<int>{};
  panicking::expect_eq(t.capacity(), 0U);

  t.reserve(100);
  panicking::expect_true(t.capacity() >= 100);

  // Should not reallocate when inserting within capacity
  auto cap_before = t.capacity();
  for (auto i = 0; i < 50; ++i) {
    t.insert(i);
  }
  panicking::expect_eq(t.capacity(), cap_before);
}

SFC_TEST(duplicate_inserts) {
  auto t = HashSet<int>{};

  // Insert same value multiple times
  for (auto i = 0; i < 10; ++i) {
    auto result = t.insert(42);
    if (i == 0) {
      panicking::expect_true(result);  // First insert
    } else {
      panicking::expect_false(result);  // Subsequent inserts
    }
  }

  panicking::expect_eq(t.len(), 1U);
  panicking::expect_true(t.contains(42));
}

}  // namespace sfc::collections::hash_set::test
