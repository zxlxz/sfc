#pragma once

#include <nms/core.h>

namespace nms::math
{

/* liner */
template<class T, u32 N>
struct Norm
{
    using TxN = Vec<T, N>;
    static constexpr auto $rank = N;

    Norm(const T(&step)[N])
        : scal_(step)
    {}

    static constexpr u32 rank() {
        return N;
    }

    template<class I>
    u32 size(I idx) const noexcept {
        return 0;
    }

    template<class ...I>
    T operator()(I ...ids) const {
        static_assert(u32(sizeof...(I)) == N, "unexpect arguments count, should be N");

        T idx[] = { T(ids)... };
        T nrm2  = 0;
        for (u32 k = 0; k < N; ++k) {
            nrm2 += (idx[k] * idx[k] * scal_[k] * scal_[k]);
        }
        T result = T(sqrt(nrm2));
        return result;
    }

private:
    TxN scal_;
};

template<class T, u32 N>
constexpr auto toLambda(const Norm<T, N>& v) {
    return v;
}

template<class T, class ...U>
constexpr auto vnorm(const T& t, const U& ...u) {
    return Norm<T, u32(1 + sizeof...(U))>({ T(t), T(u)... });
}

}
