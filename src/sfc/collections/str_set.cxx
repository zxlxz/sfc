#include "sfc/test.h"
#include "sfc/collections/str_set.h"

namespace sfc::collections::str_set::test {

SFC_TEST(insert_search) {
  auto set = StrSet::new_();

  // empty set
  sfc::assert_eq(set.search(Str{"a"}), Option<u32>{});

  const auto a = set.insert(Str{"a"});
  sfc::assert_eq(a.idx, 0U);
  sfc::assert_eq(a.inserted, true);

  const auto b = set.insert(Str{"bb"});
  sfc::assert_eq(b.idx, 1U);
  sfc::assert_eq(b.inserted, true);

  const auto c = set.insert(Str{"ccc"});
  sfc::assert_eq(c.idx, 2U);
  sfc::assert_eq(c.inserted, true);

  const auto d = set.insert({});
  sfc::assert_eq(d.idx, 3U);
  sfc::assert_eq(d.inserted, true);

  // duplicate insert
  const auto a2 = set.insert(Str{"a"});
  sfc::assert_eq(a2.idx, 0U);
  sfc::assert_eq(a2.inserted, false);

  // search
  sfc::assert_eq(set.search(Str{"a"}), Option{0U});
  sfc::assert_eq(set.search(Str{"bb"}), Option{1U});
  sfc::assert_eq(set.search(Str{"ccc"}), Option{2U});
  sfc::assert_eq(set.search(Str{}), Option{3U});
  sfc::assert_eq(set.search(Str{"dddd"}), Option<u32>{});
}

SFC_TEST(long_str) {
  auto set = StrSet::new_();

  const auto s1 = Str{"abcdefgh"};   // exactly kAlignSize
  const auto s2 = Str{"abcdefghij"}; // spans two slots
  const auto s3 = Str{"0123456789abcdef"};

  const auto r1 = set.insert(s1);
  const auto r2 = set.insert(s2);
  const auto r3 = set.insert(s3);
  sfc::assert_eq(r1.inserted, true);
  sfc::assert_eq(r2.inserted, true);
  sfc::assert_eq(r3.inserted, true);

  sfc::assert_eq(set.search(s1), Option{0U});
  sfc::assert_eq(set.search(s2), Option{2U});
  sfc::assert_eq(set.search(s3), Option{4U});

  sfc::assert_eq(set.search(Str{"0123456789abcde"}), Option<u32>{});
  sfc::assert_eq(set.search(Str{"0123456789abcdefX"}), Option<u32>{});
}

SFC_TEST(search_prefix) {
  auto set = StrSet::new_();

  set.insert(Str{"abcdef"});
  set.insert(Str{"ab"});

  // prefix of a longer key must not match
  sfc::assert_eq(set.search(Str{"abc"}), Option<u32>{});
  sfc::assert_eq(set.search(Str{"abcd"}), Option<u32>{});
  sfc::assert_eq(set.search(Str{"abcde"}), Option<u32>{});

  // longer key must not match a shorter one
  sfc::assert_eq(set.search(Str{"a"}), Option<u32>{});

  sfc::assert_eq(set.search(Str{"abcdef"}), Option{0U});
  sfc::assert_eq(set.search(Str{"ab"}), Option{1U});
}

SFC_TEST(with_capacity) {
  auto set = StrSet::with_capacity(8);
  const auto r = set.insert(Str{"hello"});
  sfc::assert_eq(r.idx, 0U);
  sfc::assert_eq(r.inserted, true);
  sfc::assert_eq(set.search(Str{"hello"}), Option{0U});
}

}  // namespace sfc::collections::str_set::test
