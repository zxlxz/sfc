#include "sfc/test.h"
#include "sfc/collections/str_map.h"

namespace sfc::collections::str_map::test {

SFC_TEST(insert_get) {
  auto map = StrMap<int>::new_();
  sfc::assert_eq(map.len(), 0U);
  sfc::assert_eq(map.get(Str{"a"}), Option<const int&>{});

  sfc::assert_eq(map.insert(Str{"a"}, 1), Option<int>{});
  sfc::assert_eq(map.insert(Str{"b"}, 2), Option<int>{});
  sfc::assert_eq(map.len(), 2U);

  sfc::assert_eq(map.get(Str{"a"}), Option{1});
  sfc::assert_eq(map.get(Str{"b"}), Option{2});
  sfc::assert_eq(map.get(Str{"c"}), Option<const int&>{});
}

SFC_TEST(insert_overwrite) {
  auto map = StrMap<int>::new_();
  map.insert(Str{"a"}, 1);

  const auto old = map.insert(Str{"a"}, 9);
  sfc::assert_eq(old, Option{1});

  sfc::assert_eq(map.len(), 1U);
  sfc::assert_eq(map.get(Str{"a"}), Option{9});
}

SFC_TEST(try_insert) {
  auto map = StrMap<int>::new_();

  sfc::assert_eq(map.try_insert(Str{"a"}, 1), Option<int>{});
  sfc::assert_eq(map.len(), 1U);

  // key exists, not inserted, return the value back
  const auto ret = map.try_insert(Str{"a"}, 9);
  sfc::assert_eq(ret, Option{9});
  sfc::assert_eq(map.len(), 1U);
  sfc::assert_eq(map.get(Str{"a"}), Option{1});
}

SFC_TEST(get_mut) {
  auto map = StrMap<int>::new_();
  map.insert(Str{"a"}, 1);

  {
    auto val = map.get_mut(Str{"a"});
    sfc::assert_eq(val.is_some(), true);
    *val = 5;
  }
  sfc::assert_eq(map.get(Str{"a"}), Option{5});

  auto miss = map.get_mut(Str{"b"});
  sfc::assert_eq(miss.is_some(), false);
}

}  // namespace sfc::collections::str_map::test

