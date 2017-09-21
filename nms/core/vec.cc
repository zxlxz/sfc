#include <nms/test.h>

namespace nms
{

nms_test(vec) {

    i32x4 v4 ={ 1 };
    test::assert_eq(v4[0], 1);
    test::assert_eq(v4[1], 1);
    test::assert_eq(v4[2], 1);
    test::assert_eq(v4[3], 1);

    float a3[] ={ 1.0f, 2.0f, 3.0f };
    f32x3 f3 = a3;
    test::assert_eq(f3[0], 1.0f);
    test::assert_eq(f3[1], 2.0f);
    test::assert_eq(f3[2], 3.0f);
}

}
