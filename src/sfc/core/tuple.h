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

template <u32 I>
struct idx_t {
  static constexpr auto VALUE = I;

  static auto operator[](auto&& t) noexcept -> auto& {
    static_assert(I < 16, "Tuple supports up to 16 elements");
    if constexpr (I == 0) return t._0;
    if constexpr (I == 1) return t._1;
    if constexpr (I == 2) return t._2;
    if constexpr (I == 3) return t._3;
    if constexpr (I == 4) return t._4;
    if constexpr (I == 5) return t._5;
    if constexpr (I == 6) return t._6;
    if constexpr (I == 7) return t._7;
    if constexpr (I == 8) return t._8;
    if constexpr (I == 9) return t._9;
    if constexpr (I == 10) return t._10;
    if constexpr (I == 11) return t._11;
    if constexpr (I == 12) return t._12;
    if constexpr (I == 13) return t._13;
    if constexpr (I == 14) return t._14;
    if constexpr (I == 15) return t._15;
  }
};

template <class T, T... I>
struct idxs_t {
  static void map(auto&& f) {
    (void)(f(idx_t<I>{}), ...);
  }
};

#if defined(__GNUC__) && !defined(__clang__)
template <auto N>
using seq_t = idxs_t<decltype(N), __integer_pack(N)...>;
#else
template <auto N>
using seq_t = __make_integer_seq<idxs_t, decltype(N), N>;
#endif

template <class... T>
struct Tuple : Inner<T...> {
  static constexpr u32 N = sizeof...(T);

 public:
  Tuple(T... args) noexcept : Inner<T...>{static_cast<T&&>(args)...} {}
  ~Tuple() = default;

 public:
  void map(auto&& f) const {
    seq_t<N>::map([&](auto I) { f(I[*this]); });
  }

  void map_mut(auto&& f) {
    seq_t<N>::map([&](auto I) { f(I[*this]); });
  }

  void fmt(auto& f) const {
    f.write_char('(');
    seq_t<N>::map([&](auto I) {
      if constexpr (I.VALUE != 0) f.write_str(", ");
      f.write_val(I[*this]);
    });
    f.write_char(')');
  }
};

template <class... T>
Tuple(T...) -> Tuple<T...>;

}  // namespace sfc::tuple

namespace sfc {
using tuple::Tuple;
}  // namespace sfc
