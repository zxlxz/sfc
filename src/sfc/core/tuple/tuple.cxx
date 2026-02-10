#include "sfc/test/test.h"
#include "sfc/core/tuple.h"

namespace sfc::tuple::test {

SFC_TEST(tuple_get) {
  const auto t = Tuple{1, 2.0, Str{"abc"}};
  sfc::expect_eq(t.get<0>(), 1);
  sfc::expect_eq(t.get<1>(), 2.0);
  sfc::expect_eq(t.get<2>(), Str{"abc"});
}

SFC_TEST(tuple_unbind) {
  const auto t = Tuple{1, 2.0, Str{"abc"}};
  auto [a, b, c] = t;
  sfc::expect_eq(a, 1);
  sfc::expect_eq(b, 2.0);
  sfc::expect_eq(c, Str{"abc"});
}

}  // namespace sfc::tuple::test
