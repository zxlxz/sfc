
#ifdef HAS_OPENBLAS
#include <cblas.h>
#define openblas_do(f)      cblas_##f

#include <nms/math.h>
#include <nms/test.h>

#include <nms/math/avx.h>

namespace nms::math::blas
{

enum class Vendor {
    NmsMath,
    OpenBlas,
};

#pragma region blas v1

template<Vendor VID>
float sdot(const View<float, 1>& vx, const View<float, 1>& vy) {
    float ret = 0.f;
    switch (VID) {
    case Vendor::NmsMath: {
        ret <<= vsum(vx*vy);
        break;
    }
    case Vendor::OpenBlas:
        ret = openblas_do(sdot)(i32(vx.count()), vx.data(), vx.step(0), vy.data(), vy.step(0));
        break;
    }
    return ret;
}

template<Vendor VID>
float asum(const View<float,1>& v) {
    float ret = 0.f;
    switch (VID) {
    case Vendor::NmsMath:
        ret <<= vsum(vabs(v));
        break;

    case Vendor::OpenBlas:
        ret = openblas_do(sasum)(i32(v.count()), v.data(), v.step(0));
        break;
    }

    return ret;
}

template<Vendor VID>
float nrm2(const View<float,1>& v) {
    float ret = 0.f;

    switch (VID) {
    case Vendor::NmsMath:
        ret <<= vsum(v*v);
        ret = sqrt(ret);
        break;

    case Vendor::OpenBlas:
        ret = openblas_do(snrm2)(i32(v.count()), v.data(), v.step(0));
        break;
    }

    return ret;
}

template<Vendor VID>
float axpy(float alpha, View<float, 1>& vx, View<float, 1>& vy) {
    vx <<= 1.f;
    vy <<= 1.f;

    switch (VID) {
    case Vendor::NmsMath:
        vy += alpha*vx;
        break;

    case Vendor::OpenBlas:
        openblas_do(saxpy)(i32(vx.count()), alpha, vx.data(), vx.step(0), vy.data(), vy.step(0) );
        break;
    }

    return vx(10)+vy(10);
}

#define blas_test(f, ...)                                                                                   \
    [&] {                                                                                                   \
        io::log::info("---------------");                                                                   \
        io::log::info("{}({})", #f, #__VA_ARGS__);                                                          \
        for(auto i = 0; i < 4; ++i ) {                                                                      \
            auto o0 = nms::clock(); auto oret = f<Vendor::OpenBlas>(__VA_ARGS__); auto o1 = nms::clock();   \
            auto n0 = nms::clock(); auto nret = f<Vendor::NmsMath >(__VA_ARGS__); auto n1 = nms::clock();   \
            io::log::info("   openblas {} {.3}", (o1-o0)*3, oret);                                          \
            io::log::info("   nms.math {} {.3}", (n1-n0)*3, nret);                                          \
        }                                                                                                   \
    }()

nms_test(test_1s) {
    u32 count = 1 * 1024 * 1024;

    Array<float, 1> vx({ count });
    Array<float, 1> vy({ count });

    vx <<= 1.f;
    vy <<= 2.f;

    blas_test(sdot, vx, vy);
    blas_test(asum, vx);
    blas_test(nrm2, vx);
    blas_test(axpy, 1.f, vx, vy);
}

#pragma endregion

}

#endif
