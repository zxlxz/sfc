#pragma once

#include <nms/core.h>

namespace nms::math
{

template<class X>        struct Scalar;
template<class X>        struct Eye;
template<class T, u32 N> struct Linespace;

template<class F, class ...Ts> struct Parallel;
template<class F, class ...Ts> struct Reduce;

#pragma region scalar
template<class X>
struct Scalar
{
    static constexpr auto $rank = 0u;

    Scalar(const X& x)
        : x_(x) {
    }

    static constexpr auto rank() {
        return $rank;
    }

    template<class I>
    u32 size(I idx) const noexcept {
        return 1u;
    }

    template<class ...I>
    const X& operator()(I ...idx) const noexcept {
        return x_;
    }

protected:
    X   x_;
};

/* cast Scalar<T> to Parallel */
template<class X>
auto toLambda(const Scalar<X>& x) {
    return x;
}

template<class T>
auto toScalar(const T& t) -> decltype($when_is<$number, T>{}, Scalar<T>{}) {
    return { t };
}

/* mk scalar */
template<class T>
Scalar<T> mkScalar(const T& t) {
    return { t };
}

#pragma endregion


#pragma region Parallel
template<class F, class X>
struct Parallel<F, X>
{
    static constexpr const auto $rank = X::$rank;

    Parallel(const X& x)
        : x_(x) {
    }

    static constexpr auto rank() {
        return $rank;
    }

    template<class I>
    auto size(I idx) const noexcept {
        return x_.size(idx);
    }

    template<class ...I>
    auto operator()(I ...idx) const noexcept {
        return F::run(x_(idx...));
    }

protected:
    X   x_;
};

template<class F, class X, class Y>
struct Parallel<F, X, Y>
{
    static constexpr auto $rank = X::$rank > Y::$rank ? X::$rank : Y::$rank;
    static constexpr F    $func = {};

    Parallel(const X& x, const Y& y)
        : x_(x), y_(y) {
        static_assert(X::$rank == Y::$rank || X::$rank == 0 || Y::$rank == 0, "nms.math.Parallel: $rank not match");
    }

    static constexpr auto rank() {
        return $rank;
    }

    template<class I>
    auto size(I idx) const noexcept {
        const auto sx = x_.size(idx);
        const auto sy = y_.size(idx);
        return X::$rank == 0 ? sy : Y::$rank == 0 ? sx : nms::min(sx, sy);
    }

    template<class ...I>
    auto operator()(I ...idx) const noexcept {
        return F::run(x_(idx...), y_(idx...));
    }

protected:
    X   x_;
    Y   y_;
};

/* cast Parallel<F(...)> to Parallel */
template<class F, class ...T>
auto toLambda(const Parallel<F, T...>& val) {
    return val;
}

/* make Parallel<F(x)> */
template<class F, class X>
auto mkParallel(const X& x) -> Parallel<F, decltype(lambda_cast(x))> {
    return { lambda_cast(x) };
}

/* make Parallel<F(A,B)> */
template<class F, class X, class Y>
auto mkParallel(const X& x, const Y& y) -> Parallel<F, decltype(lambda_cast(x)), decltype(lambda_cast(y))> {
    return { lambda_cast(x), lambda_cast(y) };
}
#pragma endregion

#pragma region Reduce
template<class F, class X>
struct Reduce<F, X>
{
    static constexpr auto   $rank = X::$rank - 1;

    Reduce(const X& x) noexcept
        : x_(x) {
    }

    static constexpr auto rank() {
        return $rank;
    }

    template<class I>
    auto size(I idx) const noexcept {
        return x_.size(idx + 1);
    }

    template<class ...I>
    auto operator()(I ...idx) const {
        const auto  n = x_.size(0);

        if (n < 2) {
            return x_(0, idx...);
        }

        auto ret = F::run(x_(0, idx...), x_(1, idx...));
        for (u32 i = 2; i < n; ++i) {
            ret = F::run(ret, x_(i, idx...));
        }

        return ret;
    }

private:
    X   x_;
};

/* cast Reduce<F(...)> to Parallel */
template<class F, class ...T>
auto toLambda(const Reduce<F, T...>& val) {
    return val;
}

template<class F, class X>
auto mkReduce(const X& x) -> Reduce<F, decltype(lambda_cast(x))> {
    return { lambda_cast(x) };
}

#pragma endregion

#pragma region lambda_cast
// lambda_cast: -> Scalar
template<class X>
auto _lambda_cast(const X& x, const void*) -> decltype(Scalar<X>{x}) {
    return Scalar<X>{x};
}

// lambda_cast: -> toLambda
template<class X>
auto _lambda_cast(const X& x, const X*) -> decltype(toLambda(x)) {
    return toLambda(x);
}

template<class X>
auto lambda_cast(const X& x) -> decltype(_lambda_cast(x, &x)) {
    return _lambda_cast(x, &x);
}
#pragma endregion

#pragma region testLambda2
template<class X, class Y>
constexpr auto _testLambda2(const X& x, const Y&y, Version<2>) ->decltype(toLambda(x), toLambda(y), 0 ) {
    return 0;
}

template<class X, class Y, class=$when_is<$number, Y> >
constexpr auto _testLambda2(const X& x, const Y&y, Version<1>) ->decltype(toLambda(x), 0) {
    return 0;
}

template<class X, class Y, class=$when_is<$number, X> >
constexpr auto _testLambda2(const X& x, const Y&y, Version<0>) ->decltype(toLambda(y), 0) {
    return 0;
}

template<class X, class Y>
constexpr auto testLambda2(const X& x, const Y& y) -> decltype(_testLambda2(x, y, Version<2>{})) {
    return 0;
}
#pragma endregion

#pragma region math operators

/* parallel: +x */
template<class X>
constexpr auto operator+(const X& x) noexcept
    ->decltype(toLambda(x), mkParallel<Pos>(x)) 
{
    return mkParallel<Pos>(x);
}

/* parallel: -x */
template<class X>
constexpr auto operator-(const X& x) noexcept 
    ->decltype(toLambda(x), mkParallel<Neg>(x))
{
    return mkParallel<Neg>(x);
}

/* parallel: a+b */
template<class X, class Y>
constexpr auto operator+(const X& x, const Y& y) noexcept 
    -> decltype(testLambda2(x, y), mkParallel<Add>(x, y) )
{
    return mkParallel<Add>(x, y);
}

/* parallel: a-b */
template<class X, class Y>
constexpr auto operator-(const X& x, const Y& y) noexcept 
    -> decltype(testLambda2(x, y), mkParallel<Sub>(x, y) ) 
{
    return mkParallel<Sub>(x, y); 
}

/* parallel: a*b */
template<class X, class Y>
constexpr auto operator*(const X& x, const Y& y) noexcept 
    -> decltype(testLambda2(x, y), mkParallel<Mul>(x, y) ) 
{
    return mkParallel<Mul>(x, y); 
}

/* parallel: a/b */
template<class X, class Y>
constexpr auto operator/(const X& x, const Y& y) noexcept 
    -> decltype(testLambda2(x, y), mkParallel<Div>(x, y) ) 
{
    return mkParallel<Div>(x, y); 
}

/* parallel: a^b */
template<class X, class Y>
constexpr auto operator^(const X& x, const Y& y) noexcept 
    -> decltype(testLambda2(x, y), mkParallel<Pow>(x, y) ) 
{
    return mkParallel<Pow>(x, y); 
}

#pragma endregion

#pragma region logic operators
/* parallel: a==b */
template<class A, class B>
constexpr auto operator==(const A& a, const B& b) noexcept
    -> decltype(testLambda2(a, b), mkParallel< Eq>(a, b))
{
    return mkParallel< Eq>(a, b);
}

/* parallel: a!=b */
template<class A, class B>
constexpr auto operator!=(const A& a, const B& b) noexcept
    -> decltype(testLambda2(a, b), mkParallel<Neq>(a, b))
{
    return mkParallel<Neq>(a, b);
}

/* parallel: a<b */
template<class A, class B>
constexpr auto operator< (const A& a, const B& b) noexcept
    -> decltype(testLambda2(a, b), mkParallel< Lt>(a, b))
{
    return mkParallel< Lt>(a, b);
}

/* parallel: a>b */
template<class A, class B>
constexpr auto operator> (const A& a, const B& b) noexcept
    -> decltype(testLambda2(a, b), mkParallel< Gt>(a, b))
{
    return mkParallel< Gt>(a, b);
}

/* parallel: a<=b */
template<class A, class B>
constexpr auto operator<=(const A& a, const B& b) noexcept
    -> decltype(testLambda2(a, b), mkParallel< Le>(a, b))
{
    return mkParallel< Le>(a, b);
}

/* parallel: a>=b */
template<class A, class B>
constexpr auto operator>=(const A& a, const B& b) noexcept
    -> decltype(testLambda2(a, b), mkParallel< Ge>(a, b))
{
    return mkParallel< Ge>(a, b);
}

#pragma endregion

#pragma region math parallel functions

/* parallel: abs(x) */
template<class X> constexpr auto vabs  (const X& x) noexcept { return mkParallel<Abs>(x);   }

/* parallel: sqrt(x) */
template<class X> constexpr auto vsqrt (const X& x) noexcept { return mkParallel<Sqrt>(x);  }

/* parallel: x^2 */
template<class X> constexpr auto vpow2 (const X& x) noexcept { return mkParallel<Pow2>(x);  }

/* parallel: e^x */
template<class X> constexpr auto vexp  (const X& x) noexcept { return mkParallel<Exp>(x);   }

/* parallel: ln(x) */
template<class X> constexpr auto vln   (const X& x) noexcept { return mkParallel<Ln>(x);    }

/* parallel: log10(x) */
template<class X> constexpr auto vlog10(const X& x) noexcept { return mkParallel<Log10>(x); }

/* parallel: sin(x) */
template<class X> constexpr auto vsin  (const X& x) noexcept { return mkParallel<Sin>(x);   }

/* parallel: cos(x) */
template<class X> constexpr auto vcos  (const X& x) noexcept { return mkParallel<Cos>(x);   }

/* parallel: tan(x) */
template<class X> constexpr auto vtan  (const X& x) noexcept { return mkParallel<Tan>(x);   }

/* parallel: asin(x) */
template<class X> constexpr auto vasin (const X& x) noexcept { return mkParallel<Asin>(x);  }

/* parallel: acos(x) */
template<class X> constexpr auto vacos (const X& x) noexcept { return mkParallel<Acos>(x);  }

/* parallel: atan(x) */
template<class X> constexpr auto vatan (const X& x) noexcept { return mkParallel<Atan>(x);  }

#pragma endregion

#pragma region math reduce functions

/* reduce: count(x) */
template<class X>
constexpr auto vcount(const X& x)
    -> decltype(toLambda(x), mkReduce<Add>(x))
{
    return mkReduce<Add>(x);
}


/* reduce: sum(x) */
template<class X>
constexpr auto vsum(const X& x) 
    -> decltype(toLambda(x), mkReduce<Add>(x))
{
    return mkReduce<Add>(x);
}

/* reduce: min(x) */
template<class X> 
constexpr auto vmin(const X& x) 
    -> decltype(toLambda(x), mkReduce<Min>(x))
{
    return mkReduce<Min>(x);
}

/* reduce: max(x) */
template<class X> 
constexpr auto vmax(const X& x) 
    -> decltype(toLambda(x), mkReduce<Max>(x))
{
    return mkReduce<Max>(x);
}

#pragma endregion

struct Ass2{ template<class Y, class X> auto operator()(Y& y, const X& x) noexcept { return y  = x; } };
struct Add2{ template<class Y, class X> auto operator()(Y& y, const X& x) noexcept { return y += x; } };
struct Sub2{ template<class Y, class X> auto operator()(Y& y, const X& x) noexcept { return y -= x; } };
struct Mul2{ template<class Y, class X> auto operator()(Y& y, const X& x) noexcept { return y *= x; } };
struct Div2{ template<class Y, class X> auto operator()(Y& y, const X& x) noexcept { return y /= x; } };


#pragma region trans

template<class T, u32 N, class V>
struct Trans
{
    using TxN = Vec<T, N>;
    static constexpr auto $rank = N;

    constexpr Trans(TxN k, V v)
        : k_(k), v_(v)
    {}

    template<class ...Is>
    auto operator()(Is... ids) const noexcept {
        const TxN i = { T(ids)... };
        return at(i, Seq<sizeof...(Is)>{});
    }

private:
    TxN k_;
    V   v_;

    template<u32 ...K>
    auto at(const TxN& i, U32<K...>) const noexcept {
        return v_( (i[K]+k_[K])... );
    }
};

template<class T, u32 N, class V>
constexpr auto toLambda(const Trans<T, N, V>& v) {
    return v;
}

template<class T, u32 N>
struct TransHelper
{
    using TxN = Vec<T, N>;

public:
    template<class ...Ts>
    constexpr TransHelper(const Ts& ...ts)
        : k_{ts...}
    {}

    template<class V>
    constexpr auto operator| (V v) const noexcept {
        return mkTrans(v);
    }

private:
    TxN k_;

    template<class V>
    constexpr Trans<T, N, V> mkTrans (V v) const noexcept {
        return { k_, v };
    }

};

template<class I, class ...Is>
TransHelper<I,1+sizeof...(Is)> vtrans(I i, Is ...is) {
    return { i, is... };
}
#pragma endregion


#pragma region trans

template<class T, u32 N, class V>
struct Scale
{
    using TxN = Vec<T, N>;
    static constexpr auto $rank = N;

    constexpr Scale(TxN k, V v)
        : k_(k), v_(v)
    {}

    template<class ...Is>
    auto operator()(Is... ids) const noexcept {
        const TxN i = { T(ids)... };
        return at(i, Seq<sizeof...(Is)>{});
    }

private:
    TxN k_;
    V   v_;

    template<u32 ...K>
    auto at(const TxN& i, U32<K...>) const noexcept {
        return v_((i[K] * k_[K])...);
    }
};

template<class T, u32 N, class V>
constexpr auto toLambda(const Scale<T, N, V>& v) {
    return v;
}

template<class T, u32 N>
struct ScaleHelper
{
    using TxN = Vec<T, N>;

public:
    template<class ...Ts>
    constexpr ScaleHelper(const Ts& ...ts)
        : k_{ ts... }
    {}

    template<class V>
    constexpr auto operator| (V v) const noexcept {
        return mkScale(v);
    }

private:
    TxN k_;

    template<class V>
    constexpr Scale<T, N, V> mkScale(V v) const noexcept {
        return { k_, v };
    }

};

template<class I, class ...Is>
ScaleHelper<I, 1 + sizeof...(Is)> vscale(I i, Is ...is) {
    return { i, is... };
}
#pragma endregion

}

