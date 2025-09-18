#include "sfc/collections/vecmap.h"

#include "sfc/test.h"

namespace sfc::collections::test {

SFC_TEST(vecmap_basic) {
  auto map = VecMap<int, Str>{};
  panicking::expect_true(map.is_empty());
  panicking::expect_eq(map.len(), 0);

  map.insert(1, "one");
  map.insert(2, "two");
  map.insert(3, "three");

  panicking::expect_false(map.is_empty());
  panicking::expect_eq(map.len(), 3);

  panicking::expect_eq(map.get(1), Option<Str>{"one"});
  panicking::expect_eq(map.get(2), Option<Str>{"two"});
  panicking::expect_eq(map.get(3), Option<Str>{"three"});
  panicking::expect_false(map.get(4));
}

SFC_TEST(vecmap_move) {
  auto map1 = VecMap<int, Str>{};
  map1.insert(1, "one");
  map1.insert(2, "two");

  auto map2 = mem::move(map1);
  panicking::expect_eq(map2.len(), 2);
  panicking::expect_eq(map2.get(1), Option<Str>{"one"});
  panicking::expect_eq(map2.get(2), Option<Str>{"two"});
}

SFC_TEST(vecmap_try_insert) {
  auto map = VecMap<int, Str>{};
  map.insert(1, "one");
  map.insert(2, "two");

  panicking::expect_eq(map.len(), 2);
  panicking::expect_eq(map.try_insert(1, "ONE"), Option<Str>{"one"});
  panicking::expect_eq(map.try_insert(3, "three").is_none(), true);
  panicking::expect_eq(map.len(), 3);
  panicking::expect_eq(map.get(1), Option<Str>{"one"});
  panicking::expect_eq(map.get(3), Option<Str>{"three"});
}

SFC_TEST(vecmap_remove) {
  auto map = VecMap<int, Str>{};
  map.insert(1, "one");
  map.insert(2, "two");
  map.insert(3, "three");

  panicking::expect_eq(map.len(), 3);
  panicking::expect_eq(map.remove(2), Option<Str>{"two"});
  panicking::expect_eq(map.len(), 2);
  panicking::expect_false(map.get(2));

  panicking::expect_eq(map.remove(4).is_none(), true);
  panicking::expect_eq(map.len(), 2);
}

}  // namespace sfc::collections::test
