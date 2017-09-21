#pragma once

#include <nms/math/base.h>
#include <nms/math/view.h>

namespace nms::math
{

#pragma region fureach-runutor
struct Vrun
{
    template<class Tfunc, class Tret, class ...Targs>
    void foreach(Tfunc func, Tret& ret, const Targs& ...args) {
        _foreach(Tu32<Tret::$rank>{}, func, ret, args...);
    }

protected:
    template<class Tfunc, class Tret, class Targ>
    void _foreach(Tu32<0>, Tfunc, Tret& ret, const Targ& arg) {
        Tfunc::run(ret(), arg());
    }

    template<class Tfunc, class Tret, class Targ>
    void _foreach(Tu32<1>, Tfunc, Tret& ret, const Targ& arg) {
        const auto size = ret.size();
        const auto s0   = i32(size[0]);

        for (i32 i0 = 0; i0 < s0; ++i0) {
            Tfunc::run(ret(i0), arg(i0));
        }
    }

    template<class Tfunc, class Tret, class Targ>
    void _foreach(Tu32<2>, Tfunc, Tret& ret, const Targ& arg) {
        const auto size = ret.size();

        for (u32 i1 = 0; i1 < size[1]; ++i1) {
            for (u32 i0 = 0; i0 < size[0]; ++i0) {
                Tfunc::run(ret(i0, i1), arg(i0, i1));
            }
        }
    }

    template<class Tfunc, class Tret, class Targ>
    void _foreach(Tu32<3>, Tfunc, Tret& ret, const Targ& arg) {
        const auto size = ret.size();

        for (u32 i2 = 0; i2 < size[2]; ++i2) {
            for (u32 i1 = 0; i1 < size[1]; ++i1) {
                for (u32 i0 = 0; i0 < size[0]; ++i0) {
                    Tfunc::run(ret(i0, i1, i2), arg(i0, i1, i2));
                }
            }
        }
    }

    template<class Tfunc, class Tret, class Targ>
    void _foreach(Tu32<4>, Tfunc, Tret& ret, const Targ& arg) {
        const auto size = ret.size();

        for (u32 i3 = 0; i3 < size[3]; ++i3) {
            for (u32 i2 = 0; i2 < size[2]; ++i2) {
                for (u32 i1 = 0; i1 < size[1]; ++i1) {
                    for (u32 i0 = 0; i0 < size[0]; ++i0) {
                        Tfunc::run(ret(i0, i1, i2, i3), arg(i0, i1, i2, i3));
                    }
                }
            }
        }
    }
};

/* combine runutor */
inline Vrun operator||(const Vrun&, const Vrun&) {
    return {};
}

template<class T>
auto _mk_vrun(const T&, Tver<0>) -> Vrun {
    return {};
}

template<class T>
auto _mk_vrun(const T&, Tver<1>) -> typename T::Tvrun {
    return {};
}

template<class T>
auto get_vrun(const T& t) {
    return _mk_vrun(t, Tver<1>{});
}

template<class T, class U, class ...R>
auto get_vrun(const T& t, const U& u, const R& ...r) {
    return get_vrun(t) || get_vrun(u, r...);
}

template<class T>
bool check_size(const T& /*t*/) {
    return true;
}

template<class T, class U, class ...R>
bool check_size(const T& t, const U& u, const R& ...r) {
    static_assert(T::$rank == U::$rank || U::$rank == 0, "nms.math._check_size: $rank not match");

    for (u32 i = 0; i < T::$rank; ++i) {
        if (u.size(i) != 0 && u.size(i) > t.size(i)) {
            return false;
        }
    }
    return check_size(t, r...);
}

template<class Tfunc, class Tret, class ...Targs>
bool foreach(Tfunc func, Tret& ret, const Targs& ...args) {
    if (!check_size(view_cast(ret), view_cast(args)...)) {
        return false;
    }

    get_vrun(ret, args...).foreach(func, static_cast<typename Tret::Tview&>(ret), view_cast(args)...);
    return true;
}

#pragma endregion

#pragma region functions

#define NMS_IVIEW_FOREACH(op, type)                     \
    template<class T, class = typename T::Tview>        \
    constexpr auto operator op(const T& t) noexcept {   \
            return math::mkParallel<type>(t);           \
    }
NMS_IVIEW_FOREACH(+, Pos)
NMS_IVIEW_FOREACH(-, Neg)
#undef NMS_IVIEW_FOREACH

#define NMS_IVIEW_FOREACH(op, type)                                     \
    template<class X, class Y, class = decltype(view_test_xy<X, Y>())>  \
    constexpr auto operator op(const X& x, const Y& y) noexcept {       \
        return math::mkParallel<type>(x, y);                            \
    }

NMS_IVIEW_FOREACH(+ , Add)
NMS_IVIEW_FOREACH(- , Sub)
NMS_IVIEW_FOREACH(* , Mul)
NMS_IVIEW_FOREACH(/ , Div)
NMS_IVIEW_FOREACH(^ , Pow)

NMS_IVIEW_FOREACH(== ,  Eq)
NMS_IVIEW_FOREACH(!= , Neq)
NMS_IVIEW_FOREACH(<,   Lt)
NMS_IVIEW_FOREACH(>,   Gt)
NMS_IVIEW_FOREACH(<=,  Le)
NMS_IVIEW_FOREACH(>=,  Ge)
#undef NMS_IVIEW_FOREACH

template<class X, class Y, class=typename Y::Tview>
Y& operator<<=(Y& y, const X& x) {
#ifndef NMS_CC_INTELLISENSE
    foreach(Ass2{}, y, x);
#endif
    return y;
}

template<class X, class Y, class=typename Y::Tview>
Y& operator>>=(const X& x, Y& y) {
#ifndef NMS_CC_INTELLISENSE
    foreach(Ass2{}, y, x);
#endif
    return y;
}

template<class Y, class F, class ...T, class=$when_is<$number,Y> >
Y& operator<<=(Y& y, const Reduce<F, T...>& x) {
#ifndef NMS_CC_INTELLISENSE
    Scalar<Y> sy(y);
    foreach(Ass2{}, sy, x);
    y = sy();
#endif
    return y;
}

#define NMS_IVIEW_FOREACH(op, type)                     \
template<class X, class Y, class=typename Y::Tview >    \
Y& operator op(Y& y, const X& x) {                      \
    foreach(type{}, y, x);                              \
    return y;                                           \
}

NMS_IVIEW_FOREACH(+=, Add2)
NMS_IVIEW_FOREACH(-=, Sub2)
NMS_IVIEW_FOREACH(*=, Mul2)
NMS_IVIEW_FOREACH(/=, Div2)
#undef NMS_IVIEW_FOREACH

#define NMS_IVIEW_FOREACH(func, type)       \
template<class T>                           \
constexpr auto func(const T& t) noexcept {  \
    return math::mkParallel<type>(t);       \
}
NMS_IVIEW_FOREACH(vabs,    Abs)
NMS_IVIEW_FOREACH(vsqrt,   Sqrt)
NMS_IVIEW_FOREACH(vpow2,   Pow2)
NMS_IVIEW_FOREACH(vexp,    Exp)
NMS_IVIEW_FOREACH(vln,     Ln)
NMS_IVIEW_FOREACH(vlog10,  Log10)

NMS_IVIEW_FOREACH(vsin,    Sin)
NMS_IVIEW_FOREACH(vcos,    Cos)
NMS_IVIEW_FOREACH(vtan,    Tan)

NMS_IVIEW_FOREACH(vasin,   Atan)
NMS_IVIEW_FOREACH(vacos,   Acos)
NMS_IVIEW_FOREACH(vatan,   Atan)
#undef NMS_IVIEW_FOREACH

#define NMS_IVIEW_REDUCE(func, type)        \
template<class T>                           \
constexpr auto func(const T& t) noexcept {  \
    return math::mkReduce<type>(t);         \
}
NMS_IVIEW_REDUCE(vsum,      Add)
NMS_IVIEW_REDUCE(vmax,      Max)
NMS_IVIEW_REDUCE(vmin,      Min)
#undef  NMS_IVIEW_REDUCE

}
