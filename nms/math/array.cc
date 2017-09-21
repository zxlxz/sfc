
#include <nms/test.h>
#include <nms/math.h>
#include <nms/io.h>

namespace nms::math
{

nms_test(array_1d) {
    Array<f32, 1> a;// ({ 8 });
    a.resize({ 8 });
    test::assert_eq(a.size(), { 8u });
    a <<= vline(1.0f);
    for (auto i = 0u; i < a.size(0); i++) {
        test::assert_eq(a(i), f32(i));
    }

    auto b = a.dup();
    test::assert_eq(b.size(), { 8u });

    for (auto i = 0u; i < b.size(0); i++) {
        test::assert_eq(a(i), b(i));
    }

    b.resize({ 4 });
    test::assert_eq(b.size(), { 4u });
}

nms_test(array_2d) {
    Array<f32, 2> a({ 4, 8 });
    test::assert_eq(a.size(), { 4u, 8u });
    a <<= vline(1.0f, 0.1f);
    for (auto i = 0u; i < a.size(0); i++) {
        for (auto j = 0u; j < a.size(1); j++) {
            test::assert_eq(a(i, j), f32(i) + f32(j)*0.1f);
        }
    }

    auto b = a.dup();
    test::assert_eq(b.size(), { 4u, 8u });

    for (auto i = 0u; i < b.size(0); i++) {
        for (auto j = 0u; j < b.size(1); j++) {
            test::assert_eq(a(i, j), b(i, j));
        }
    }

    auto c = b.permute({ 1u, 0u });
    test::assert_eq(c.size(), { b.size(1), b.size(0) });
    for (auto i = 0u; i < b.size(0); i++) {
        for (auto j = 0u; j < b.size(1); j++) {
            test::assert_eq(b(i, j), c(j, i));
        }
    }
}

nms_test(array_math) {
    // a = zeros(32, 32)
    // b = zeros(32, 32)
    Array<f32, 2> a({ 32u, 32u });
    Array<f32, 2> b({ 32u, 32u });

    // b = |  0.0   0.1   0.2   0.3 ...  1.0 ...
    //     |  1.0   1.1   1.2   1.3 ...  1.1 ...
    //     | ...                             ...
    //     | 10.0  10.1  10.2  10.3 ... 11.0 ...
    //     | ...                             ...
    b <<= vline(0.1f, 1.f);

    // a = sin(b).*2 + cos(b)
    a <<= vsin(b) * 2 + vcos(b);

    // h:= host memory
    math::Array<f32, 2> h({ 32u, 32u });

    // h = a  (copy from device to host)
    h <<= a;

    io::console::writeln("result = {:-8.3}", h.slice({ 0u, 8u }, { 0u, 8u }));
}

nms_test(array_project3d) {
    Array<f32, 3> imag({ 10u, 64u, 64u });
    Array<f32, 2> view({ 64u, 64u });

    imag <<= vline(0.f, 0.01f, 1.f);
    view <<= vsum(imag);

    auto x_imag = imag.slice({ 0u }, { 0u, 8u }, { 0u, 8u });
    auto x_view = view.slice({ 0u, 8u }, { 0u, 8u });

    io::console::writeln("imag = {:-7.3}", x_imag);
    io::console::writeln("view = {:-7.3}", x_view);
}

nms_test(array_save_load) {
    auto a = Array<f32, 2> ({ 8, 8 });
    a <<= vline(1.0f, 0.1f);
    a.save("nms.math.array.dat");

    auto b = Array<f32, 2> ::load("nms.math.array.dat");
    test::assert_eq(a.size(), b.size());
    for (u32 i = 0; i < a.size(0); i++) {
        for (u32 j = 0; j < a.size(1); j++) {
            test::assert_eq(a(i, j), b(i, j));
        }
    }
}

}
