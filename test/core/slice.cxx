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
    auto iter = v.iter().rev();
    test::assert_eq(iter.next(), 3);
    test::assert_eq(iter.next(), 2);
  }
}

SFC_TEST(min_max) {
  int v_arr[] = {0, 1, 2, 3};
  auto v = Slice<int>{v_arr};

  test::assert_eq(v.min(), 0);
  test::assert_eq(v.max(), 3);
}

SFC_TEST(min_max_by_key) {
  int v_arr[] = {0, 1, 2, 3};
  auto v = Slice<int>{v_arr};
  test::assert_eq(v.min_by_key([](auto x) { return -x; }), 3);
  test::assert_eq(v.max_by_key([](auto x) { return -x; }), 0);
}

}  // namespace sfc::slice
