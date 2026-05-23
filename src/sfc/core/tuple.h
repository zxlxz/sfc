#pragma once

#include "sfc/core/mod.h"

namespace sfc::tuple {

template <class... T>
struct Inner;

// clang-format off
template <>
struct Inner<>{ };

template <class A>
struct Inner<A>{ A _0; };

template <class A, class B>
struct Inner<A,B>{ A _0; B _1;};

template <class A, class B, class C>
struct Inner<A,B,C> { A _0; B _1; C _2; };

template <class A, class B, class C, class D>
struct Inner<A,B,C,D> { A _0; B _1; C _2; D _3; };

template <class A, class B, class C, class D, class E>
struct Inner<A,B,C,D,E> { A _0; B _1; C _2; D _3; E _4; };

template <class A, class B, class C, class D, class E, class F>
struct Inner<A,B,C,D,E,F> { A _0; B _1; C _2; D _3; E _4; F _5; };

template <class A, class B, class C, class D, class E, class F, class G>
struct Inner<A,B,C, D,E,F,G> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; };

template <class A, class B, class C, class D, class E, class F, class G, class H>
struct Inner<A,B,C,D,E,F,G,H> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; };

template <class A, class B, class C, class D, class E, class F, class G, class H, class I>
struct Inner<A,B,C,D,E,F,G,H,I> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; };

template <class A, class B, class C, class D, class E, class F, class G, class H, class I, class J>
struct Inner<A,B,C,D,E,F,G,H,I,J> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; };

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K>
struct Inner<A,B,C,D,E,F,G,H,I,J,K> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; };

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L>
struct Inner<A,B,C,D,E,F,G,H,I,J,K,L> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11; };

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M>
struct Inner<A,B,C,D,E,F,G,H,I,J,K,L,M> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11; M _12; };

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N>
struct Inner<A,B,C,D,E,F,G,H,I,J,K,L,M,N> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11; M _12; N _13; };

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O>
struct Inner<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11; M _12; N _13; O _14; };

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P>
struct Inner<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11; M _12; N _13; O _14; P _15; };
// clang-format on

template <class... T>
struct Tuple : Inner<T...> {
  static constexpr u32 N = sizeof...(T);

 public:
  Tuple(T... args) noexcept : Inner<T...>{(T&&)(args)...} {}
  ~Tuple() = default;

 public:
  void map(auto&& f) const {
    if constexpr (N > 0) f(this->_0);
    if constexpr (N > 1) f(this->_1);
    if constexpr (N > 2) f(this->_2);
    if constexpr (N > 3) f(this->_3);
    if constexpr (N > 4) f(this->_4);
    if constexpr (N > 5) f(this->_5);
    if constexpr (N > 6) f(this->_6);
    if constexpr (N > 7) f(this->_7);
    if constexpr (N > 8) f(this->_8);
    if constexpr (N > 9) f(this->_9);
    if constexpr (N > 10) f(this->_10);
    if constexpr (N > 11) f(this->_11);
    if constexpr (N > 12) f(this->_12);
    if constexpr (N > 13) f(this->_13);
    if constexpr (N > 14) f(this->_14);
    if constexpr (N > 15) f(this->_15);
  }

  void map_mut(auto&& f) {
    if constexpr (N > 0) f(this->_0);
    if constexpr (N > 1) f(this->_1);
    if constexpr (N > 2) f(this->_2);
    if constexpr (N > 3) f(this->_3);
    if constexpr (N > 4) f(this->_4);
    if constexpr (N > 5) f(this->_5);
    if constexpr (N > 6) f(this->_6);
    if constexpr (N > 7) f(this->_7);
    if constexpr (N > 8) f(this->_8);
    if constexpr (N > 9) f(this->_9);
    if constexpr (N > 10) f(this->_10);
    if constexpr (N > 11) f(this->_11);
    if constexpr (N > 12) f(this->_12);
    if constexpr (N > 13) f(this->_13);
    if constexpr (N > 14) f(this->_14);
    if constexpr (N > 15) f(this->_15);
  }

  void fmt(auto& f) const {
    f.write_char('(');
    this->map([&](const auto& val) {
      if (val != this->_0) f.write_str(", ");
      f.write_val(val);
    });
    f.write_char(')');
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
