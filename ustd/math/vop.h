#pragma once

#include <ustd/core.h>

namespace ustd::math::vop
{

template<typename T>
struct Scalar
{
    using $MathVopView = Scalar;
    constexpr static const auto $rank = 0;

    T   _t;

    fn dims(u32 i) const noexcept -> u32 {
        (void)i;
        return 0;
    }

    template<typename ...I>
    __forceinline fn operator()(I ...) const noexcept -> const T& {
        return _t;
    }

    template<typename ...I>
    __forceinline fn operator()(I ...) noexcept  -> T& {
        return _t;
    }
};

template<typename T, u32 N>
struct Linspace
{
    using $MathVopView = Linspace;
    constexpr static const auto $rank = N;

    T _step[N];

    fn dims(u32 i) const noexcept -> u32 {
        (void)i;
        return 0;
    }

    template<typename ...Tidx>
    fn operator()(Tidx ...idxs) const -> T {
        static_assert(sizeof...(Tidx) == $rank, "ustd::math::vop::Linespace: unexpect arguments count, should be `$rank`");
        return this->get_value($index_seq<N>{}, idxs...);
    }

private:
    template<usize ...Idim, typename ...Tidx>
    fn get_value($usize<Idim...>, Tidx ...idxs) const -> T {
        return ustd::sum((T(_step[Idim])*idxs)...);
    }
};

template<typename T, typename ...U>
constexpr auto vline(const T& t, const U& ...u) {
    return Linspace<T, u32(1 + sizeof...(U))>{ { T{ t }, T{ u }... }};
}

template<typename F, typename ...T>
struct Parallel;

template<typename F, typename T>
struct Parallel<F, T>
{
    using $MathVopView = Parallel;   // trait
    constexpr static const auto $rank = T::$rank;

    T   _t;

    fn dims(u32 i) const noexcept -> u32 {
        return _t.dims(i);
    }

    template<typename ...I>
    __forceinline auto operator()(I ...idx) const noexcept {
        return F::run(_t(idx...));
    }
};

template<typename F, typename A, typename B>
struct Parallel<F, A, B>
{
    using $MathVopView = Parallel;
    constexpr static const auto $rank = A::$rank | B::$rank;

    A  _a;
    B  _b;

    fn dims(u32 i) const noexcept -> u32 {
        return A::$rank == 0 ? _a.dims(i) : _b.dims(i);
    }

    template<typename ...I>
    __forceinline auto operator()(I ...idx) const noexcept {
        return F::run(_a(idx...), _b(idx...));
    }
};

template<typename F, typename ...T>
struct Reduce;

template<typename F, typename T>
struct Reduce<F, T>
{
    constexpr static const auto $rank = T::$rank - 1;

    using $MathVopView = Reduce;

    T   _t;

    fn dims(u32 i) const noexcept -> u32 {
        return _t.dims(i + 1);
    }

    template<typename ...I>
    fn operator()(I ...i) const {
        let cnt = _t.dims(0);

        mut ret = F::run(_t(0, i...), _t(1, i...));

        for (u32 k = 2u; k < cnt; ++k) {
            ret = F::run(ret, vt(k, i...));
        }
        return ret;
    }
};

struct Runner
{
    template<typename F, class R, class ...T>
    fn foreach(F func, R& res, const T& ...args) -> void {
        _foreach($u32<R::$rank>{}, func, res, args...);
    }

    friend fn operator||(Runner, Runner) -> Runner {
        return {};
    }
protected:
    template<typename F, class R, class T>
    fn _foreach($u32<0>, F, R& res, const T& arg) -> void {
        F::run(res(), arg());
    }

    template<typename F, class R, class T>
    fn _foreach($u32<1>, F, R& res, const T& arg) -> void {
        let nx = res.dims(0);

        for (u32 ix = 0; ix < nx; ++ix) {
            F::run(res(ix), arg(ix));
        }
    }

    template<typename F, class R, class T>
    fn _foreach($u32<2>, F, R& res, const T& arg) -> void {
        let nx = res.dims(0);
        let ny = res.dims(1);

        for (u32 iy = 0; iy < ny; ++iy) {
            for (u32 ix = 0; ix < nx; ++ix) {
                F::run(res(ix, iy), arg(ix, iy));
            }
        }
    }

    template<typename F, class R, class T>
    fn _foreach($u32<3>, F, R& res, const T& arg) -> void {
        let nx = res.dims(0);
        let ny = res.dims(1);
        let nz = res.dims(2);

        for (u32 iz = 0; iz < nz; ++iz) {
            for (u32 iy = 0; iy < ny; ++iy) {
                for (u32 ix = 0; ix < nx; ++ix) {
                    F::run(res(ix, iy, iz), arg(ix, iy, iz));
                }
            }
        }
    }

    template<typename F, class R, class T>
    void _foreach($u32<4>, F, F& res, const T& arg) {
        let nx = res.dims(0);
        let ny = res.dims(1);
        let nz = res.dims(2);
        let nw = res.dims(3);

        for (u32 iw = 0; iw < nw; ++iw) {
            for (u32 iz = 0; iz < nz; ++iz) {
                for (u32 iy = 0; iy < ny; ++iy) {
                    for (u32 ix = 0; ix < nx; ++ix) {
                        F::run(res(ix, iy, iz, iw), arg(ix, iy, iz, iw));
                    }
                }
            }
        }
    }
};

namespace detail
{

template<typename T>
fn _to_view(const T& t, $ver<1>) -> typename T::$MathVopView {
    return t;
}

template<typename T, typename = $when_is<$number, T> >
fn _to_view(const T& t, $ver<0>) -> Scalar<T> {
    return { t };
}

template<typename T>
fn to_view(const T&t) {
    return _to_view(t, $ver<1>{});
}

template<typename A, typename B, typename = typename A::$MathVopView, typename = typename B::$MathVopView>
fn _view_test2($ver<2>) -> void
{}

template<typename A, typename B, typename = typename A::$MathVopView, typename = $when_is<$number, B> >
fn _view_test2($ver<1>) -> void
{}

template<typename A, typename B, typename = $when_is<$number, A>, typename = typename B::$MathVopView>
fn _view_test2($ver<0>) -> void
{}

template<typename A, typename B>
fn view_test2() -> decltype(_view_test2<A, B>($ver<2>{}))
{}

template<typename F, typename A>
fn make_parallel(const A& a) -> Parallel<F, decltype(to_view(a))> {
    return { a };
}

template<typename F, typename A, typename B>
fn make_parallel(const A& a, const B& b) -> Parallel<F, decltype(to_view(a)), decltype(to_view(b)) > {
    return { a, b };
}

template<typename F, typename T>
fn make_reduce(const T& t) -> Reduce<F, decltype(to_view(t)) > {
    return { t };
}

template<class T>
fn _get_runner(const T&, $ver<0>) -> Runner {
    return {};
}

template<class T>
fn _get_runner(const T&, $ver<1>) -> typename T::VRunner {
    return {};
}

template<class T>
fn get_runner(const T& t) {
    return _get_runner(t, $ver<1>{});
}

template<class T, class U, class ...R>
fn get_runner(const T& t, const U& u, const R& ...r) {
    return get_runner(t) || get_runner(u, r...);
}

template<class T>
fn check_dims(const T& /*t*/) -> bool {
    return true;
}

template<class T, class U, class ...R>
fn check_dims(const T& t, const U& u, const R& ...r) -> bool {
    static_assert(T::$rank == U::$rank || U::$rank == 0, "ustd.math.check_size: $rank not match");

    for (u32 i = 0; i < T::$rank; ++i) {
        if (u.dims(i) != 0 && u.dims(i) > t.dims(i)) {
            return false;
        }
    }
    return check_dims(t, r...);
}

}

template<typename F, class R, class ...T>
fn foreach(F func, R& ret, const T& ...args) -> bool {
    using namespace detail;

    if (!detail::check_dims(to_view(ret), to_view(args)...)) {
        return false;
    }

    detail::get_runner(ret, args...).foreach(func, (typename R::$MathVopView&)(ret), detail::to_view(args)...);
    return true;
}

#pragma region functions

#define USTD_MATH_VOP_FOREACH(op, F)                        \
    struct F {                                              \
        template<class T>                                   \
        static fn run(T t) noexcept {                       \
            return op t;                                    \
        }                                                   \
    };                                                      \
    template<class T, class = typename T::$MathVopView>     \
    fn operator op(const T& t) noexcept {                   \
        return detail::make_parallel<F>(t);                 \
    }
USTD_MATH_VOP_FOREACH(+, Pos)
USTD_MATH_VOP_FOREACH(-, Neg)
#undef USTD_MATH_VOP_FOREACH

#define USTD_MATH_VOP_FOREACH(op, F)                                            \
    struct F {                                                                  \
        template<class A, class B>                                              \
        static fn run(A a, B b) noexcept {                                      \
            return a op b;                                                      \
        }                                                                       \
    };                                                                          \
    template<class A, class B, class = decltype(detail::view_test2<A, B>())>    \
    fn operator op(const A& a, const B& b) noexcept {                           \
        return detail::make_parallel<F>(a, b);                                  \
    }

USTD_MATH_VOP_FOREACH(+, Add)
USTD_MATH_VOP_FOREACH(-, Sub)
USTD_MATH_VOP_FOREACH(*, Mul)
USTD_MATH_VOP_FOREACH(/, Div)
USTD_MATH_VOP_FOREACH(^, Pow)

USTD_MATH_VOP_FOREACH(==, Eq)
USTD_MATH_VOP_FOREACH(!=, Neq)
USTD_MATH_VOP_FOREACH(< , Lt)
USTD_MATH_VOP_FOREACH(> , Gt)
USTD_MATH_VOP_FOREACH(<=, Le)
USTD_MATH_VOP_FOREACH(>=, Ge)
#undef USTD_MATH_VOP_FOREACH

struct Fassign {
    template<class R, class T>
    static fn run(R& r, T t) noexcept -> R& {
        return r = t;
    }
}; 

template<class R, class T, class = typename R::$MathVopView>
fn operator<<=(R& res, const T& arg) -> R& {
    foreach(Fassign{}, res, arg);
    return res;
}

template<class R, class T, class = typename R::$MathVopView>
fn operator>>=(const T& arg, R& res) -> R& {
    foreach(Fassign{}, res, arg);
    return res;
}

template<class R, class F, class ...T, class = $when_is<$number, R> >
fn operator<<=(R& ref, const Reduce<F, T...>& arg) -> R& {
    Scalar<R> tmp = { ref };
    foreach(Fassign{}, tmp, arg);
    ref = tmp._t;
    return ref;
}

#define USTD_MATH_VOP_FOREACH(op, F)                                \
struct F {                                                          \
    template<class R, class T>                                      \
    static fn run(R& r, T t) noexcept -> R& {                       \
        return r op t;                                              \
    }                                                               \
};                                                                  \
template<class R, class T, class=typename R::$MathVopView >         \
fn operator op(R& res, const T& arg) -> R& {                        \
    foreach(F{}, res, arg);                                         \
    return res;                                                     \
}

USTD_MATH_VOP_FOREACH(+=, Fadd_to)
USTD_MATH_VOP_FOREACH(-=, Fsub_to)
USTD_MATH_VOP_FOREACH(*=, Fmul_to)
USTD_MATH_VOP_FOREACH(/=, Fdiv_to)
#undef USTD_MATH_VOP_FOREACH

#define USTD_MATH_VOP_FOREACH(op)                                   \
struct F##op {                                                      \
    template<class T>                                               \
    static fn run(T x) noexcept {                                   \
        return op(x);                                               \
    }                                                               \
};                                                                  \
template<class T>                                                   \
fn v##op(const T& t) noexcept {                                     \
    return detail::make_parallel<F##op>(t);                         \
}

USTD_MATH_VOP_FOREACH(abs)
USTD_MATH_VOP_FOREACH(sqrt)
USTD_MATH_VOP_FOREACH(pow2)
USTD_MATH_VOP_FOREACH(exp)
USTD_MATH_VOP_FOREACH(ln)
USTD_MATH_VOP_FOREACH(log10)

USTD_MATH_VOP_FOREACH(sin)
USTD_MATH_VOP_FOREACH(cos)
USTD_MATH_VOP_FOREACH(tan)

USTD_MATH_VOP_FOREACH(asin)
USTD_MATH_VOP_FOREACH(acos)
USTD_MATH_VOP_FOREACH(atan)
#undef USTD_MATH_VOP_FOREACH

#define USTD_MATH_VOP_REDUCE(op, F)                             \
struct F                                                        \
{                                                               \
    template<class T>                                           \
    static fn run(T& a, T& b)  noexcept -> T& {                 \
        return a > b ? a : b;                                   \
    }                                                           \
};                                                              \
template<class T>                                               \
fn op(const T& t) noexcept {                                    \
    return detail::make_reduce<F>(t);                           \
}
USTD_MATH_VOP_REDUCE(vmax,      Fmax)
USTD_MATH_VOP_REDUCE(vmin,      Fmin)
#undef  USTD_MATH_VOP_REDUCE

}
