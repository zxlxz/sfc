#include "sfc/test/test.h"
#include "sfc/core/tuple.h"

namespace sfc::tuple::test {

SFC_TEST(visit) {
  const auto t = Tuple{1, 2.0, Str{"abc"}};
  sfc::assert_eq(t._0, 1);
  sfc::assert_eq(t._1, 2.0);
  sfc::assert_eq(t._2, Str{"abc"});
}

SFC_TEST(tuple_bind) {
  auto a = 1;
  auto b = 2.0;
  auto c = Str{"abc"};
  auto t = bind(a, b, c);
  static_assert(trait::same_<decltype(t), Tuple<int&, double&, Str&>>);
  sfc::assert_eq(t._0, 1);
  sfc::assert_eq(t._1, 2.0);
  sfc::assert_eq(t._2, Str{"abc"});
}

SFC_TEST(tuple_struct) {
  const auto t = Tuple{1, 2.0};
  auto [a, b] = t;
  sfc::assert_eq(a, 1);
  sfc::assert_eq(b, 2.0);
}

SFC_TEST(tuple_fmt) {
  const auto t = Tuple{1, 2, 3};
  sfc::assert_eq(string::format("{}", t), "(1, 2, 3)");
}

}  // namespace sfc::tuple::test
