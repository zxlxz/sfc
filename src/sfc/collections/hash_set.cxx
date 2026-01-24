#include "sfc/collections/hash_set.h"

#include "sfc/test/test.h"

namespace sfc::collections::hash_set::test {

SFC_TEST(contains) {
  auto t = HashSet<int>{};
  panicking::expect(!t.contains(1));

  t.insert(1);
  panicking::expect(t.contains(1));
  panicking::expect(!t.contains(2));

  t.insert(2);
  t.insert(3);
  panicking::expect(t.contains(1));
  panicking::expect(t.contains(2));
  panicking::expect(t.contains(3));
  panicking::expect(!t.contains(4));
}

SFC_TEST(insert) {
  auto t = HashSet<int>{};

  // First insert should return true (newly inserted)
  panicking::expect(t.insert(1));
  panicking::expect_eq(t.len(), 1U);
  panicking::expect(t.contains(1));

  // Second insert of same value should return false (already exists)
  panicking::expect(!t.insert(1));
  panicking::expect_eq(t.len(), 1U);  // Length unchanged

  // Insert new values
  panicking::expect(t.insert(2));
  panicking::expect(t.insert(3));
  panicking::expect_eq(t.len(), 3U);

  // Verify all values present
  panicking::expect(t.contains(1));
  panicking::expect(t.contains(2));
  panicking::expect(t.contains(3));
}

SFC_TEST(remove) {
  auto t = HashSet<int>{};

  // Remove from empty set
  panicking::expect(!t.remove(1));

  // Add some values
  for (auto i = 0; i < 3; ++i) {
    t.insert(i);
  }
  panicking::expect_eq(t.len(), 3U);

  // Remove existing values
  panicking::expect(t.remove(0));
  panicking::expect(!t.contains(0));
  panicking::expect_eq(t.len(), 2U);

  panicking::expect(t.remove(1));
  panicking::expect(!t.contains(1));
  panicking::expect_eq(t.len(), 1U);

  panicking::expect(t.remove(2));
  panicking::expect(!t.contains(2));
  panicking::expect_eq(t.len(), 0U);

  // Remove already removed value
  panicking::expect(!t.remove(2));
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
    panicking::expect(!t.contains(i));
  }

  // Can insert again after clear
  t.insert(42);
  panicking::expect(t.contains(42));
  panicking::expect_eq(t.len(), 1U);
}

SFC_TEST(with_capacity) {
  auto t = HashSet<int>::with_capacity(50);
  panicking::expect_eq(t.len(), 0U);
  panicking::expect(t.capacity() >= 50);

  for (auto i = 0; i < 100; ++i) {
    t.insert(i);
  }
  panicking::expect_eq(t.len(), 100U);
}

SFC_TEST(reserve) {
  auto t = HashSet<int>{};
  panicking::expect_eq(t.capacity(), 0U);

  t.reserve(100);
  panicking::expect(t.capacity() >= 100);

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
      panicking::expect(result);  // First insert
    } else {
      panicking::expect(!result);  // Subsequent inserts
    }
  }

  panicking::expect_eq(t.len(), 1U);
  panicking::expect(t.contains(42));
}

}  // namespace sfc::collections::hash_set::test
