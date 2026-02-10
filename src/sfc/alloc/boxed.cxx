#include "sfc/test/test.h"

namespace sfc::boxed::test {

SFC_TEST(obj) {
  auto b1 = Box<int>::xnew(42);
  sfc::expect_eq(*b1, 42);

  auto b2 = Box<int>::xnew(43);
  sfc::expect_eq(*b2, 43);

  mem::swap(b1, b2);
  sfc::expect_eq(*b1, 43);
  sfc::expect_eq(*b2, 42);
}

SFC_TEST(fun) {
  auto f1 = [](int x) { return 10 * x; };

  auto b1 = Box<int(int)>::xnew(f1);
  sfc::expect_eq(b1(1), 10);

  auto b2 = mem::move(b1);
  sfc::expect_eq(b2(2), 20);
}

}  // namespace sfc::boxed::test
