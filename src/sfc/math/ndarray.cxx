#include "sfc/math/ndarray.h"

#include "sfc/test.h"

namespace sfc::math::test {

SFC_TEST(NdArray_WithShape) {
  // 1d
  {
    auto arr = NdArray<int, 1>::with_shape({6});
    sfc::expect_eq(arr.numel(), 6);
  }

  // 2d
  {
    u32 shape[2] = {3, 3};
    auto arr = NdArray<int, 2>::with_shape(shape);
    sfc::expect_eq(arr.numel(), 9);
  }

  // 3d
  {
    u32 shape[3] = {2, 3, 4};
    auto arr = NdArray<int, 3>::with_shape(shape);
    sfc::expect_eq(arr.numel(), 24);
  }
}

SFC_TEST(NdArray_AsView) {
  // 1d
  {
    u32 shape[1] = {6};
    auto arr = NdArray<int, 1>::with_shape(shape);
    auto view = arr.as_view();
    sfc::expect_eq(view._shape[0], 6);
  }

  // 2d
  {
    u32 shape[2] = {3, 3};
    auto arr = NdArray<int, 2>::with_shape(shape);
    auto view = arr.as_view();
    sfc::expect_eq(view._shape, (u32[]){3, 3});
  }
}

SFC_TEST(NdArray_Access) {
  // 1d
  {
    u32 shape[1] = {6};
    auto arr = NdArray<int, 1>::with_shape(shape);
    auto ptr = arr.as_mut_ptr();
    for (auto i = 0U; i < 6; ++i) {
      ptr[i] = static_cast<int>(i);
    }

    auto view = arr.as_view();
    sfc::expect_eq(view[0], 0);
    sfc::expect_eq(view[1], 1);
    sfc::expect_eq(view[5], 5);
  }

  // 2d
  {
    // [0, 1, 2]
    // [3, 4, 5]
    // [6, 7, 8]
    u32 shape[2] = {3, 3};
    auto arr = NdArray<int, 2>::with_shape(shape);
    auto ptr = arr.as_mut_ptr();
    for (auto i = 0U; i < 9; ++i) {
      ptr[i] = static_cast<int>(i);
    }

    auto view = arr.as_view();
    sfc::expect_eq(view[0, 0], 0);
    sfc::expect_eq(view[1, 0], 1);
    sfc::expect_eq(view[0, 1], 3);
    sfc::expect_eq(view[1, 1], 4);
    sfc::expect_eq(view[2, 2], 8);
  }
}

}  // namespace sfc::math::test
