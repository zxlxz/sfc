#pragma once

#include "sfc/core/mod.h"

namespace sfc::tuple {

template <class... T>
struct _TupleData;

// clang-format off
template <>
struct _TupleData<>{ };

template <class A>
struct _TupleData<A>{ A _0; };

template <class A, class B>
struct _TupleData<A, B>{ A _0; B _1;};

template <class A, class B, class C>
struct _TupleData<A, B, C> { A _0; B _1; C _2; };

template <class A, class B, class C, class D>
struct _TupleData<A, B, C, D> { A _0; B _1; C _2; D _3; };

template <class A, class B, class C, class D, class E>
struct _TupleData<A, B, C, D, E> { A _0; B _1; C _2; D _3; E _4; };

template <class A, class B, class C, class D, class E, class F>
struct _TupleData<A, B, C, D, E, F> { A _0; B _1; C _2; D _3; E _4; F _5; };

template <class A, class B, class C, class D, class E, class F, class G>
struct _TupleData<A, B, C, D, E, F, G> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; };

template <class A, class B, class C, class D, class E, class F, class G, class H>
struct _TupleData<A, B, C, D, E, F, G, H> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; };

template <class A, class B, class C, class D, class E, class F, class G, class H, class I>
struct _TupleData<A, B, C, D, E, F, G, H, I> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; };

template <class A, class B, class C, class D, class E, class F, class G, class H, class I, class J>
struct _TupleData<A, B, C, D, E, F, G, H, I, J> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; };

template <class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K>
struct _TupleData<A, B, C, D, E, F, G, H, I, J, K> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; };

template <class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L>
struct _TupleData<A, B, C, D, E, F, G, H, I, J, K, L> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11; };

template <class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M>
struct _TupleData<A, B, C, D, E, F, G, H, I, J, K, L, M> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11; M _12; };

template <class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N>
struct _TupleData<A, B, C, D, E, F, G, H, I, J, K, L, M, N> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11; M _12; N _13; };

template <class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O>
struct _TupleData<A, B, C, D, E, F, G, H, I, J, K, L, M, N, O> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11; M _12; N _13; O _14; };

template <class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P>
struct _TupleData<A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11; M _12; N _13; O _14; P _15; };
// clang-format on

void for_each(auto&& self, auto&& f) {
  if constexpr (requires { self._0; }) f(self._0);
  if constexpr (requires { self._1; }) f(self._1);
  if constexpr (requires { self._2; }) f(self._2);
  if constexpr (requires { self._3; }) f(self._3);
  if constexpr (requires { self._4; }) f(self._4);
  if constexpr (requires { self._5; }) f(self._5);
  if constexpr (requires { self._6; }) f(self._6);
  if constexpr (requires { self._7; }) f(self._7);
  if constexpr (requires { self._8; }) f(self._8);
  if constexpr (requires { self._9; }) f(self._9);
  if constexpr (requires { self._10; }) f(self._10);
  if constexpr (requires { self._11; }) f(self._11);
  if constexpr (requires { self._12; }) f(self._12);
  if constexpr (requires { self._13; }) f(self._13);
  if constexpr (requires { self._14; }) f(self._14);
  if constexpr (requires { self._15; }) f(self._15);
}

void for_each_idx(auto&& self, auto&& f) {
  if constexpr (requires { self._0; }) f(0, self._0);
  if constexpr (requires { self._1; }) f(1, self._1);
  if constexpr (requires { self._2; }) f(2, self._2);
  if constexpr (requires { self._3; }) f(3, self._3);
  if constexpr (requires { self._4; }) f(4, self._4);
  if constexpr (requires { self._5; }) f(5, self._5);
  if constexpr (requires { self._6; }) f(6, self._6);
  if constexpr (requires { self._7; }) f(7, self._7);
  if constexpr (requires { self._8; }) f(8, self._8);
  if constexpr (requires { self._9; }) f(9, self._9);
  if constexpr (requires { self._10; }) f(10, self._10);
  if constexpr (requires { self._11; }) f(11, self._11);
  if constexpr (requires { self._12; }) f(12, self._12);
  if constexpr (requires { self._13; }) f(13, self._13);
  if constexpr (requires { self._14; }) f(14, self._14);
  if constexpr (requires { self._15; }) f(15, self._15);
}

template <class... T>
struct Tuple : _TupleData<T...> {
  static constexpr u32 COUNT = sizeof...(T);
  using Inn = _TupleData<T...>;

 public:
  Tuple(T... args) noexcept : Inn{(T&&)(args)...} {}
  ~Tuple() = default;

 public:
  void for_each(auto&& f) const {
    tuple::for_each(*this, f);
  }

  void for_each_mut(auto&& f) {
    tuple::for_each(*this, f);
  }

  void fmt(auto& f) const {
    auto imp = f.debug_tuple("");
    tuple::for_each(*this, [&](const auto& val) { imp.field(val); });
  }
};

template <class... T>
Tuple(T...) -> Tuple<T...>;

template <class... T>
auto bind(T&... args) -> Tuple<T&...> {
  return Tuple<T&...>{args...};
}

}  // namespace sfc::tuple

namespace sfc {
using tuple::Tuple;
}  // namespace sfc
