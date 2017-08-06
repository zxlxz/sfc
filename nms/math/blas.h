#pragma once

#include <nms/math/base.h>
#include <nms/math/lambda.h>

namespace nms::math
{

inline namespace blas
{

template<class F, class R, class V>
R _reduce(const V& v) {
    auto f = mkReduce<F>(v);
    R r;
    r = f();
    return r;
}

/**
 * get the maximum value
 */
template<class T>
T (max)(const View<T, 1>& view) {
    return blas::_reduce<Max, T>(view);
}

/**
 * get the minimum value
 */
template<class T>
T (min)(const View<T, 1>& view) {
    return blas::_reduce<Min, T>(view);
}

/**
* get the sum value
*/
template<class T>
T sum(const View<T, 1>& view) {
    return blas::_reduce<Add, T>(view);
}

}

}
