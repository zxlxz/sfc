#include "sfc/test.h"

namespace sfc::boxed {

SFC_TEST(box_obj) {
  auto b1 = Box<int>::xnew(42);
  panicking::assert_eq(*b1, 42);

  auto b2 = Box<int>::xnew(43);
  panicking::assert_eq(*b2, 43);

  mem::swap(b1, b2);
  panicking::assert_eq(*b1, 43);
  panicking::assert_eq(*b2, 42);

  auto b3 = Box<int>::from_raw(new int{44});
  panicking::assert_eq(*b3, 44);

  auto b4 = mem::move(b3);
  panicking::assert_false(b3);
  panicking::assert_true(b4);
  panicking::assert_eq(*b4, 44);
}

SFC_TEST(box_fn) {
  auto f1 = [](int x) { return x + 1; };

  auto b1 = Box<int(int)>::xnew(f1);
  panicking::assert_eq(b1(41), 42);

  auto b2 = mem::move(b1);
  panicking::assert_eq(b2(21), 22);
  panicking::assert_false(bool(b1));
}

}  // namespace sfc::boxed
