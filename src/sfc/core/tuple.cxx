#include "sfc/test/test.h"
#include "sfc/core/tuple.h"

namespace sfc::tuple::test {

SFC_TEST(visit) {
  const auto t = Tuple{1, 2.0, Str{"abc"}};
  sfc::assert_eq(t._0, 1);
  sfc::assert_eq(t._1, 2.0);
  sfc::assert_eq(t._2, Str{"abc"});
}

SFC_TEST(bind) {
  auto a = 1;
  auto b = 2.0;
  auto c = Str{"abc"};
  auto t = bind(a, b, c);
  static_assert(trait::same_<decltype(t), Tuple<int&, double&, Str&>>);
  sfc::assert_eq(t._0, 1);
  sfc::assert_eq(t._1, 2.0);
  sfc::assert_eq(t._2, Str{"abc"});
}

SFC_TEST(unpack) {
  const auto t = Tuple{1, 2.0};
  auto [a, b] = t;
  sfc::assert_eq(a, 1);
  sfc::assert_eq(b, 2.0);
}

SFC_TEST(fmt) {
  const auto t = Tuple{1, 2, 3};
  sfc::assert_eq(string::format("{}", t), "(1, 2, 3)");
}

SFC_TEST(for_each) {
  const auto t = Tuple{1, 2, 3};

  auto sum = 0;
  t.for_each([&](const auto& val) { sum += val; });
  sfc::assert_eq(sum, 6);
}

SFC_TEST(fold) {
  const auto t = Tuple{1, 2, 3};
  auto result = t.fold(0, [](auto acc, auto val) { return acc + val; });
  sfc::assert_eq(result, 6);
}

}  // namespace sfc::tuple::test
