#include "sfc/test/test.h"
#include "sfc/core/tuple.h"

namespace sfc::tuple::test {

SFC_TEST(tuple_visit) {
  const auto t = Tuple{1, 2.0, Str{"abc"}};
  sfc::expect_eq(t._0, 1);
  sfc::expect_eq(t._1, 2.0);
  sfc::expect_eq(t._2, Str{"abc"});
}

SFC_TEST(tuple_map) {
  const auto t = Tuple{1, 2, 3};
  auto sum = 0;
  t.map([&sum](auto x) { sum += x; });
  sfc::expect_eq(sum, 6);
}

SFC_TEST(tuple_fmt) {
  const auto t = Tuple{1, 2, 3};
  sfc::expect_eq(string::format("{}", t), "(1, 2, 3)");
}

}  // namespace sfc::tuple::test
