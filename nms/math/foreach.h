#pragma once

#include <nms/core.h>
#include <nms/math/lambda.h>
#include <nms/io/log.h>

namespace nms::math
{

struct ForeachExecutor
{
    /**
     * foreach element:
     * dst = fun(src)
     */
    template<class Tfunc, class Tret, class ...Targs>
    void run(Tfunc fun, Tret& ret, const Targs& ...args) {
        foreach_exec(U32<Tret::$rank>{}, fun, ret, args...);
    }

protected:
    /**
     * foreach element:
     * ret = func(arg)
     *
     * when: ret.$rank = 1
     */
    template<class Tfunc, class Tret, class Targ>
    void foreach_exec(U32<1>, Tfunc func, Tret& ret, const Targ& arg) {
        const auto size = ret.size();

        for (u32 i0 = 0; i0 < size[0]; ++i0) {
            func(ret(i0), arg(i0));
        }
    }

    /**
     * foreach element:
     * ret = func(arg)
     *
     * when: ret.$rank = 2
     */
    template<class Tfunc, class Tret, class Targ>
    void foreach_exec(U32<2>, Tfunc func, Tret& ret, const Targ& arg) {
        const auto size = ret.size();

        for (u32 i1 = 0; i1 < size[1]; ++i1) {
            for (u32 i0 = 0; i0 < size[0]; ++i0) {
                func(ret(i0, i1), arg(i0, i1));
            }
        }
    }

    /**
     * foreach element:
     * ret = func(arg)
     *
     * when: ret.$rank = 3
     */
    template<class Tfunc, class Tret, class Targ>
    void foreach_exec(U32<3>, Tfunc func, Tret& ret, const Targ& arg) {
        const auto size = ret.size();

        for (u32 i2 = 0; i2 < size[2]; ++i2) {
            for (u32 i1 = 0; i1 < size[1]; ++i1) {
                for (u32 i0 = 0; i0 < size[0]; ++i0) {
                    func(ret(i0, i1, i2), arg(i0, i1, i2));
                }
            }
        }
    }

    /**
     * foreach element:
     * ret = func(src)
     *
     * when: ret.$rank = 4
     */
    template<class Tfunc, class Tret, class Targ>
    void foreach_exec(U32<4>, Tfunc fun, Tret& ret, const Targ& arg) {
        const auto size = ret.size();

        for (u32 i3 = 0; i3 < size[3]; ++i3) {
            for (u32 i2 = 0; i2 < size[2]; ++i2) {
                for (u32 i1 = 0; i1 < size[1]; ++i1) {
                    for (u32 i0 = 0; i0 < size[0]; ++i0) {
                        fun(ret(i0, i1, i2, i3), arg(i0, i1, i2, i3));
                    }
                }
            }
        }
    }
};

/* combine executor */
inline ForeachExecutor operator||(const ForeachExecutor&, const ForeachExecutor&) {
    return {};
}

/* redirect-to mkExecutor(t) */
template<class T>
auto _select_foreach_executor(const T& t, const T*) -> decltype(mkForeachExecutor(t)) {
    return mkForeachExecutor(t);
}

/* return default foreach-executor */
template<class T>
auto _select_foreach_executor(const T& t, ...) {
    return ForeachExecutor{};
}

/* get foreach-executor match {t} */
template<class T>
auto get_foreach_executor(const T& t) {
    return _select_foreach_executor(t, &t);
}

/* get foreach-executor match {t, u...} */
template<class T, class ...U>
auto get_foreach_executor(const T& t, const U& ...u) {
    return get_foreach_executor(t) || get_foreach_executor(u...) ;
}

/**
 * check:
 *   ret.$rank   == {args...}.$rank
 *   ret.size(i) == {args...}.size(i)
 */
template<class Tret, class Targ, class ...Targs>
bool _foreach_check_size(const Tret& ret, const Targ& arg, const Targs& ...args) {
    static_assert(Tret::$rank == Targ::$rank || Targ::$rank == 0, "nms.math._foreach_check_size: $rank not match");
    for (u32 i = 0; i < arg.$rank; ++i) {
        if (arg.size(i) != 0 && ret.size(i) > arg.size(i)) {
            return false;
        }
    }
    return true;
}

template<class Tret>
bool _foreach_check_size(const Tret& ret) {
    return true;
}


/**
 * foreach element:
 * y = func(x)
 *
 * when: y type is View<T,N>
 */
template<class Tfunc, class Tret, class ...Targs>
bool foreach(Tfunc func, Tret& ret, const Targs& ...args) {
    auto executor = get_foreach_executor(ret, args...);

    if (!_foreach_check_size(ret, args...)) {
        io::log::error("nms.math.foreach: check size failed");
        return false;
    }

    auto lret = lambda_cast(ret);
    executor.run(func, lret, lambda_cast(args)...);
    return true;
}

/**
 * foreach
 */
template<class T, u32 N>
void _foreach_check_runable(View<T, N>&)
{}

/**
 * foreach element:
 * y <== x
 */
template<class X, class Y>
auto operator<<=(Y& y, const X& x) -> decltype(_foreach_check_runable(y), y)& {
    foreach(Ass2{}, y, x);
    return y;
}

/**
 * foreach element:
 * y >>= x
 */
template<class X, class Y>
auto operator>>=(const X& x, Y& y) -> decltype(_foreach_check_runable(y), y)& {
    foreach(Ass2{}, y, x);
    return y;
}

}
