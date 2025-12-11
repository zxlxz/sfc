#include "sfc/test/ut.h"

namespace sfc::boxed::test {

SFC_TEST(box_obj) {
  auto b1 = Box<int>::xnew(42);
  panicking::expect_eq(*b1, 42);

  auto b2 = Box<int>::xnew(43);
  panicking::expect_eq(*b2, 43);

  mem::swap(b1, b2);
  panicking::expect_eq(*b1, 43);
  panicking::expect_eq(*b2, 42);
}

SFC_TEST(box_fn) {
  auto f1 = [](int x) { return 10 * x; };

  auto b1 = Box<int(int)>::xnew(f1);
  panicking::expect_eq(b1(1), 10);

  auto b2 = mem::move(b1);
  panicking::expect_eq(b2(2), 20);
}

SFC_TEST(box_fn_copy) {
  int x1 = 1;
  int x2 = 2;

  auto f1 = [&x1](int x) { return 10 * x + x1; };
  auto f2 = [x1, x2](int x) { return 10 * x + x1 + x2; };
  auto f3 = [&x1, &x2](int x) { return 10 * x + x1 + x2; };

  {
    auto a1 = Box<int(int)>::xnew(f1);
    panicking::expect_eq(a1(1), 11);

    auto b2 = mem::move(a1);
    panicking::expect(!bool(a1));
  }

  {
    auto a2 = Box<int(int)>::xnew(f2);
    panicking::expect_eq(a2(1), 13);

    auto b2 = mem::move(a2);
    panicking::expect_eq(b2(1), 13);
  }

  {
    auto a3 = Box<int(int)>::xnew(f3);
    panicking::expect_eq(a3(1), 13);

    auto b3 = mem::move(a3);
    panicking::expect(!bool(a3));
    panicking::expect_eq(b3(1), 13);
  }
}

}  // namespace sfc::boxed::test
