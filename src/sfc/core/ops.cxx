#include "sfc/test.h"

namespace sfc::ops::test {

SFC_TEST(range) {
  const auto r = Range{0U, 10U};
  sfc::assert_eq(r._start, 0U);
  sfc::assert_eq(r._end, 10U);
  sfc::assert_eq(r.len(), 10U);

  const auto r2 = Range{5U, End{}};
  sfc::assert_eq(r2._start, 5U);
  sfc::assert_eq(r2._end, num::Int<u32>::MAX);
}

SFC_TEST(range_wrap) {
  const auto a = Range{0U, 10U}.wrap(5U);
  sfc::assert_eq(a._start, 0U);
  sfc::assert_eq(a._end, 5U);
  sfc::assert_eq(a.len(), 5U);

  const auto b = Range{10U, 20U}.wrap(5U);
  sfc::assert_eq(b._start, 5U);
  sfc::assert_eq(b._end, 5U);
  sfc::assert_eq(b.len(), 0U);
}

SFC_TEST(range_fmt) {
  const auto r = Range{0U, 10U};
  sfc::assert_eq(string::format("{}", r), "0..10");

  const auto r2 = Range{5U, $};
  sfc::assert_eq(string::format("{}", r2), "5..$");
}

SFC_TEST(range_for) {
  auto sum = 0U;
  for (auto i : Range{0U, 10U}) {
    sum += i;
  }
  sfc::assert_eq(sum, 45U);
}

}  // namespace sfc::ops::test
