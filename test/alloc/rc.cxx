#include "sfc/test.h"

namespace sfc::rc {

namespace {
struct T {
  int _val;

  static auto obj_cnt() -> int& {
    static auto res = 0;
    return res;
  }

  T(int value) : _val{value} {
    obj_cnt() += 1;
  }

  ~T() {
    obj_cnt() -= 1;
  }

  T(const T&) = delete;
};
}  // namespace

SFC_TEST(rc) {
  test::assert_eq(T::obj_cnt(), 0);
  {
    auto a = Rc<T>::xnew(1);
    test::assert_eq(a->_val, 1);
    test::assert_eq(T::obj_cnt(), 1);

    {
      auto b = a.clone();
      test::assert_eq(b->_val, 1);
      test::assert_eq(T::obj_cnt(), 1);
    }

    test::assert_eq(T::obj_cnt(), 1);
  }

  test::assert_eq(T::obj_cnt(), 0);
}

}  // namespace sfc::rc
