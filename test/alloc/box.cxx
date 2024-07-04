#include "sfc/test.h"

namespace sfc::boxed {

namespace {
struct T {
  f64 _val;

  static auto obj_cnt() -> int& {
    static auto res = 0;
    return res;
  }

  T(f64 value) : _val{value} {
    obj_cnt() += 1;
  }

  ~T() {
    obj_cnt() -= 1;
  }

  T(const T&) = delete;
};
}  // namespace

SFC_TEST(box) {
  {
    auto b1 = Box<T>::xnew(1.23);
    test::assert_eq(T::obj_cnt(), 1);
    test::assert_true(!!b1);
    test::assert_eq(b1->_val, 1.23);

    auto b2 = mem::move(b1);
    test::assert_eq(b2->_val, 1.23);
    test::assert_true(!b1);
    test::assert_eq(T::obj_cnt(), 1);
  }
  test::assert_eq(T::obj_cnt(), 0);
}

SFC_TEST(fn) {}

}  // namespace sfc::boxed
