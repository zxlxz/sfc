#pragma once

#include <nms/core.h>

namespace nms::math
{

/* liner */
template<class T, u32 N>
struct Linespace
{
    using TxN = Vec<T, N>;
    constexpr static const auto $rank = N;

    Linespace(const T(&step)[N])
        : step_(step)
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
        static_assert(u32(sizeof...(I)) == N,   "unexpect arguments count, should be N");

        const T idx[] = { T(ids)... };

        T offset = 0;
        for (u32 k = 0; k < N; ++k) {
            offset += step_[k] * idx[k];
        }
        return offset;
    }

private:
    TxN step_;
};

template<class T, u32 N>
constexpr auto toLambda(const Linespace<T, N>& v) {
    return v;
}

template<class T, class ...U>
constexpr auto vlins(const T& t, const U& ...u) {
    return Linespace<T, u32(1 + sizeof...(U))>({ T(t), T(u)... });
}

}
