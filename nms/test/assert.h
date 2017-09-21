#pragma once

#include <nms/core.h>

namespace nms::test
{

class AssertFailed
    : public IException
{
};

template<class T>
void assert_eq(const T& a, const T& b) {
    if (!(a == b)) {
        NMS_THROW(AssertFailed{});
    }
}

template<class T>
void assert_neq(const T& a, const T& b) {
    if (a == b) {
        NMS_THROW(AssertFailed{});
    }
}

inline void assert_true(bool value) {
    if (!value) {
        io::log::error("nms.test: not true", value);
        NMS_THROW(AssertFailed{});
    }
}

inline void assert_false(bool value) {
    if (value) {
        io::log::error("nms.test: not false", value);
        NMS_THROW(AssertFailed{});
    }
}

inline void assert_eq(f32 a, f32 b, f32 delta = 1e-6f) {
    if (fabs(a) < 1e-30f && fabs(b) < 1e-30f) {
        return;
    }

    int ea = 0;
    int eb = 0;

    const auto sa = ::frexpf(a, &ea);
    const auto sb = ::frexpf(b, &eb);

    if (ea != eb) {
        if (ea > -10 || eb > -10) {
            io::log::error("nms.test: {} != {}", a, b);
            NMS_THROW(AssertFailed{});
        }
    }
    else {
        auto corr = sa - sb;
        if (corr < -delta || corr > +delta) {
            io::log::error("nms.test: {} != {}", a, b);
            NMS_THROW(AssertFailed{});
        }
    }
}

inline void assert_eq( f64 a, f64 b, f64 delta = 1e-12f) {
    int ea = 0;
    int eb = 0;

    auto sa = ::frexp(a, &ea);
    auto sb = ::frexp(b, &eb);

    if (ea != eb) {
        if (ea > -20 || eb > -20) {
            io::log::error("nms.test: {} != {}", a, b);
            NMS_THROW(AssertFailed{});
        }
    }
    else {
        auto corr = sa - sb;
        if (corr < -delta || corr > +delta) {
            io::log::error("nms.test: {} != {}", a, b);
            NMS_THROW(AssertFailed{});
        }
    }
}

template<class T, u32 N>
inline void assert_eq(const Vec<T, N>& a, const Vec<T, N>& b) {
    try {
        for (u32 i = 0; i < N; ++i) {
            assert_eq(a[i], b[i]);
        }
    }
    catch (const AssertFailed&) {
        io::log::error("nms.test: {} != {}", a, b);
        throw;
    }
}

}
