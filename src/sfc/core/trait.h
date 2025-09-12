#pragma once

#include "sfc/core/mod.h"

namespace sfc::trait {

template <class T, class U>
concept same_ = __is_same(T, U);

template <class T, class... U>
concept any_ = (... || trait::same_<T, U>);

template <class T>
concept enum_ = __is_enum(T);

template <class T>
concept class_ = __is_class(T);

template <class T>
concept signed_ = any_<T, signed char, short, int, long, long long>;

template <class T>
concept unsigned_ = any_<T, unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long>;

template <class T>
concept int_ = signed_<T> || unsigned_<T>;

template <class T>
concept float_ = any_<T, float, double>;

template <class T>
concept trivially_copyable_ = __is_trivially_copyable(T);

template <class T>
concept trivially_destructible_ = __is_trivially_destructible(T);

template <class T>
auto declval() -> T&&;

template <class>
struct Expr;

template <class F, class... T>
struct Expr<F(T...)> {
  using Type = decltype(declval<F>()(declval<T>()...));
};

template <class X>
using expr_t = typename Expr<X>::Type;

}  // namespace sfc::trait
