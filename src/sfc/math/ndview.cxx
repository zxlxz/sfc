#include "sfc/math/ndview.h"

#include "sfc/test.h"

namespace sfc::math::test {

SFC_TEST(NdView_OpIdx) {
  int array[] = {0, 1, 2, 3, 4, 5, 7, 8};

  // 1d
  {
    auto v = NdView<int, 1>{array, {6}, {1}};
    sfc::expect_eq(v._shape[0], 6);
    sfc::expect_eq(v[1], 1);
  }

  // 2d
  {
    auto v = NdView<int, 2>{array, {3, 3}, {1, 3}};
    sfc::expect_eq(v._shape, (u32[]){3, 3});
    sfc::expect_eq(v[1, 1], 4);
  }
}

SFC_TEST(NdView_slice) {
  int array[] = {0, 1, 2, 3, 4, 5, 7, 8};

  // 1d
  {
    auto v = NdView<int, 1>{array, {6}, {1}};

    auto w = v[{1, 4}];
    sfc::expect_eq(w._shape, (u32[]){3});
    sfc::expect_eq(w[0], 1);
    sfc::expect_eq(w[1], 2);
    sfc::expect_eq(w[2], 3);
  }

  // 2d -> 2d
  {
    // [0, 1, 2]
    // [3, 4, 5]
    // [6, 7, 8]
    auto v = NdView<int, 2>{array, {3, 3}, {1, 3}};

    // [0, 1]
    // [3, 4]
    NdView<int, 2> w = v[{0, 1}, {0, 1}];
    sfc::expect_eq(w._shape, (u32[]){1, 1});
    sfc::expect_eq(w[0, 0], 0);
    sfc::expect_eq(w[1, 1], 4);
  }

  // 2d -> 1d
  {
    // [0, 1, 2]
    // [3, 4, 5]
    auto v = NdView<int, 2>{array, {3, 2}, {1, 3}};

    // [3, 4]
    auto w = v[{0, 2}, {1}];
    static_assert(w.NDIM == 1);
    sfc::expect_eq(w._shape, (u32[]){2});
    sfc::expect_eq(w[0], 3);
    sfc::expect_eq(w[1], 4);
  }
}

}  // namespace sfc::math::test
