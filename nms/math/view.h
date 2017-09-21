#pragma once

#include <nms/core.h>

namespace nms::math
{

#pragma region view
template<class T>
auto _view_cast(const T& t, Tver<0> ) -> Scalar<T> {
    return t;
}

template<class T>
auto _view_cast(const T& t, Tver<1> ) -> typename T::Tview {
    return t;
}

template<class T>
auto view_cast(const T&t) {
    return _view_cast(t, Tver<1>{});
}

template<class X, class Y, class= typename X::Tview, class = typename Y::Tview>
auto _view_test_xy(Tver<2>)  {
    return 0;
}

template<class X, class Y, class = typename X::Tview, class = $when<($is<$number, Y> || $is<bool, Y>)> >
auto _view_test_xy(Tver<1>)  {
    return 0;
}

template<class X, class Y, class = $when_is<$number, X>, class = typename Y::Tview>
auto _view_test_xy(Tver<0>)  {
    return 0;
}

template<class X, class Y>
auto view_test_xy() -> decltype(_view_test_xy<X, Y>(Tver<2>{})) {
    return 0;
}
#pragma endregion

#pragma region Parallel
template<class F, class ...T>
struct Parallel;

template<class F, class T>
struct Parallel<F, T>
{
    using Tview = Parallel;

    constexpr static const auto $rank = T::$rank;

    Parallel(const T& t)
        : t_(t) {}

    template<class I>
    __forceinline auto size(I idx) const noexcept {
        return t_.size(idx);
    }

    template<class ...I>
    __forceinline auto operator()(I ...idx) const noexcept {
        return F::run(t_(idx...));
    }

protected:
    T   t_;
};

template<class F, class X, class Y>
struct Parallel<F, X, Y>
{
    using Tview = Parallel;

    constexpr static const auto $rank = X::$rank | Y::$rank;

    Parallel(const X& x, const Y& y)
        : x_(x), y_(y) {}

    template<class I>
    __forceinline auto size(I idx) const noexcept {
        const auto sx = x_.size(idx);
        const auto sy = y_.size(idx);
        return X::$rank == 0 ? sy : Y::$rank == 0 ? sx : nms::min(sx, sy);
    }

    template<class ...I>
    __forceinline auto operator()(I ...idx) const noexcept {
        return F::run(x_(idx...), y_(idx...));
    }

protected:
    X   x_;
    Y   y_;
};

/* make Parallel<F(x)> */
template<class F, class X>
auto mkParallel(const X& x) {
    using Vx = decltype(view_cast(x));
    return Parallel<F, Vx>{ x };
}

/* make Parallel<F(A,B)> */
template<class F, class X, class Y>
auto mkParallel(const X& x, const Y& y) {
    using Vx = decltype(view_cast(x));
    using Vy = decltype(view_cast(y));
    return Parallel<F, Vx, Vy>{ x, y };
}

#pragma endregion

#pragma region Reduce
template<class F, class ...T>
struct Reduce;

template<class F, class X>
struct Reduce<F, X>
{
    using Tview = Reduce;
    constexpr static const auto $rank = X::$rank - 1;

    Reduce(const X& x) noexcept
        : x_(x)
    {}

    template<class I>
    auto size(I idx) const noexcept {
        return x_.size(idx + 1);
    }

    template<class ...I>
    auto operator()(I ...ids) const {
        const auto  n = x_.size(0);

        auto ret = x_(0, ids...);
        for (u32 i = 1; i < n; ++i) {
            ret = F::run(ret, x_(i, ids...));
        }
        return ret;
    }

private:
    X   x_;

};

template<class F, class X>
auto mkReduce(const X& x) -> Reduce<F, decltype(view_cast(x)) > {
    return { view_cast(x) };
}

#pragma endregion

#pragma region vline

template<class T, u32 N>
struct Vline
{
    using Tview = Vline;
    using Tstep = Vec<T, N>;
    constexpr static const auto $rank = N;

    Vline(const T(&step)[N])
        : step_(step)
    {}

    static constexpr u32 rank() {
        return N;
    }

    template<class I>
    u32 size(I /*idx*/) const noexcept {
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
    Tstep   step_;
};

template<class T, class ...U>
constexpr auto vline(const T& t, const U& ...u) {
    return Vline<T, u32(1 + sizeof...(U))>({ T(t), T(u)... });
}
#pragma endregion

#pragma region veye

template<class T>
struct Veye
{
public:
    using Tview = Veye;
    constexpr static const auto $rank = 0;

    constexpr Veye() = default;

    template<class I>
    u32 size(I /*idx*/) const noexcept {
        return 0;
    }

    template<class ...I>
    T operator()(I ...i) const noexcept {
        static_assert($all_is<$int, I...>,  "unexpect type, shold be int");
        static constexpr auto N = u32(sizeof...(I));

        const T idx[] = { T(i)... };

        auto v = idx[0];
        for (u32 k = 1; k < N; ++k) {
            if (idx[k] != v) {
                return T(0);
            }
        }
        return T(1);
    }
};

template<class T>
constexpr auto veye() {
    return Veye<T>{};
}
#pragma endregion

}
