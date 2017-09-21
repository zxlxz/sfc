#include <nms/test.h>
#include <nms/math.h>

namespace nms::math
{

nms_test(vline_1d) {
    Array<f32, 1> v({ 8 });

    v <<= vline(1.0f);
    for (u32 i = 0; i < v.size(0); ++i) {
        test::assert_eq(v(i), f32(i));
    }
}

nms_test(vline_2d) {
    Array<f32, 2> v({ 8, 8 });

    v <<= vline(1.0f, 0.1f);
    for (u32 i = 0; i < v.size(0); ++i) {
        for (u32 j = 0; j < v.size(1); ++j) {
            test::assert_eq(v(i,j), f32(i) + f32(j)*0.1f);
        }
    }
}

nms_test(vline_3d) {
    Array<f32, 3> v({ 8, 8, 8 });

    v <<= vline(1.0f, 0.1f, 0.01f);
    for (u32 i = 0; i < v.size(0); ++i) {
        for (u32 j = 0; j < v.size(1); ++j) {
            for (u32 k = 0; k < v.size(2); ++k) {
                test::assert_eq(v(i, j, k), f32(i) + f32(j)*0.1f + f32(k)*0.01f);
            }
        }
    }
}

nms_test(vreduce_1d) {

    Array<f32, 1> v({ 11 });
    v <<= vline(1.0f) - 5;

    {
        f32 min_val = 0;
        min_val <<= vmin(v);
        test::assert_eq(min_val, -5.0f);
    }


    {
        f32 min_abs = 0;
        min_abs <<= vmin(vabs(v));
        test::assert_eq(min_abs, 0.0f);
    }

    {
        f32 max_val = 0;
        max_val <<= vmax(v);
        test::assert_eq(max_val, +5.0f);
    }

    {
        f32 max_val2 = 0;
        max_val2 <<= vmax(v*v);
        test::assert_eq(max_val2, 25.0f);
    }

    {
        f32 sum_val = 0;
        sum_val <<= vsum(v);
        test::assert_eq(sum_val, 0.0f);
    }
}

}