#include "sfc/collections/vecmap.h"

#include "sfc/test.h"

namespace sfc::collections::test {

SFC_TEST(vecmap_basic) {
  auto map = VecMap<int, Str>{};
  panicking::assert_true(map.is_empty());
  panicking::assert_eq(map.len(), 0);

  map.insert(1, "one");
  map.insert(2, "two");
  map.insert(3, "three");

  panicking::assert_false(map.is_empty());
  panicking::assert_eq(map.len(), 3);

  panicking::assert_eq(map.get(1), Option{"one"});
  panicking::assert_eq(map.get(2), Option{"two"});
  panicking::assert_eq(map.get(3), Option{"three"});
  panicking::assert_false(map.get(4));
}

SFC_TEST(vecmap_move) {
  auto map1 = VecMap<int, Str>{};
  map1.insert(1, "one");
  map1.insert(2, "two");

  auto map2 = mem::move(map1);
  panicking::assert_eq(map2.len(), 2);
  panicking::assert_eq(map2.get(1), Option{"one"});
  panicking::assert_eq(map2.get(2), Option{"two"});
}

SFC_TEST(vecmap_try_insert) {
  auto map = VecMap<int, Str>{};
  map.insert(1, "one");
  map.insert(2, "two");

  panicking::assert_eq(map.len(), 2);
  panicking::assert_eq(map.try_insert(1, "uno"), Option{"one"});
  panicking::assert_eq(map.try_insert(3, "three"), Option<Str>{});
  panicking::assert_eq(map.len(), 3);
  panicking::assert_eq(map.get(1), Option{"uno"});
  panicking::assert_eq(map.get(3), Option{"three"});
}

SFC_TEST(vecmap_remove) {
  auto map = VecMap<int, Str>{};
  map.insert(1, "one");
  map.insert(2, "two");
  map.insert(3, "three");

  panicking::assert_eq(map.len(), 3);
  panicking::assert_eq(map.remove(2), Option{"two"});
  panicking::assert_eq(map.len(), 2);
  panicking::assert_false(map.get(2));

  panicking::assert_eq(map.remove(4), Option<Str>{});
  panicking::assert_eq(map.len(), 2);
}

}  // namespace sfc::collections::test
