#include "sfc/test.h"

namespace sfc::slice {

SFC_TEST(iter) {
  int v_arr[] = {0, 1, 2, 3};
  auto v = Slice<int>{v_arr};

  {
    auto iter = v.iter();
    test::assert_eq(iter.next(), 0);
    test::assert_eq(iter.next(), 1);
  }

  {
    auto iter = v.iter()->rev();
    test::assert_eq(iter.next(), 3);
    test::assert_eq(iter.next(), 2);
  }
}

}  // namespace sfc::slice
