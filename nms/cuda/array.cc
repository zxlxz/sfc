#include <nms/test.h>
#include <nms/cuda.h>

namespace nms::cuda
{

#pragma region unittest
nms_test(array) {
    // a:= device memory
    cuda::Array<f32, 2> a({ 32, 32 });

    // b:= device memory
    cuda::Array<f32, 2> b({ 32, 32 });

    // b = |  0.0   0.1   0.2   0.3 ...  1.0 ...
    //     |  1.0   1.1   1.2   1.3 ...  1.1 ...
    //     | ...                             ...
    //     | 10.0  10.1  10.2  10.3 ... 11.0 ...
    //     | ...                             ...

    b <<= vline(0.1f, 1.f);

    // a = sin(b).*2 + cos(b)
    // a <<= 2.0f * vsin(b) + vcos(b);

    // h:= host memory
    math::Array<f32, 2> h({ 32, 32 });

    // h = a  (copy from device to host)
    h <<= a;

    io::log::debug("result = {:|}", h.slice({ 0u, 8u }, { 0u, 8u }));
}

nms_test(project) {
    cuda::Array<f32, 3> imag({ 10, 64, 64});
    cuda::Array<f32, 2> view({ 64, 64 });

    imag <<= vline(0.f, 0.01f, 1.f);
    view <<= vsum(imag);

    math::Array<f32, 3> host_imag({ 10, 64, 64 });
    math::Array<f32, 2> host_view({ 64, 64 });
    host_imag <<= imag;
    host_view <<= view;

    auto x_imag = host_imag.slice({ 0u }, { 0u, 8u }, { 0u, 8u });
    auto x_view = host_view.slice({ 0u, 8u }, { 0u, 8u });

    io::log::debug("imag = {:|}", x_imag);
    io::log::debug("view = {:|}", x_view);
}
#pragma endregion

}
