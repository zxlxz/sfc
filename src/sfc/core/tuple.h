#pragma once

#include "sfc/core/trait.h"

namespace sfc::tuple {

namespace detail {
template <class... T>
struct Tuple;

// clang-format off
template <>
struct Tuple<>{ };

template <class A>
struct Tuple<A>{ A _0; };

template <class A, class B>
struct Tuple<A, B>{ A _0; B _1;};

template <class A, class B, class C>
struct Tuple<A, B, C> { A _0; B _1; C _2; };

template <class A, class B, class C, class D>
struct Tuple<A, B, C, D> { A _0; B _1; C _2; D _3; };

template <class A, class B, class C, class D, class E>
struct Tuple<A, B, C, D, E> { A _0; B _1; C _2; D _3; E _4; };

template <class A, class B, class C, class D, class E, class F>
struct Tuple<A, B, C, D, E, F> { A _0; B _1; C _2; D _3; E _4; F _5; };

template <class A, class B, class C, class D, class E, class F, class G>
struct Tuple<A, B, C, D, E, F, G> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; };

template <class A, class B, class C, class D, class E, class F, class G, class H>
struct Tuple<A, B, C, D, E, F, G, H> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; };

template <class A, class B, class C, class D, class E, class F, class G, class H, class I>
struct Tuple<A, B, C, D, E, F, G, H, I> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; };

template <class A, class B, class C, class D, class E, class F, class G, class H, class I, class J>
struct Tuple<A, B, C, D, E, F, G, H, I, J> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; };

template <class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K>
struct Tuple<A, B, C, D, E, F, G, H, I, J, K> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; };

template <class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L>
struct Tuple<A, B, C, D, E, F, G, H, I, J, K, L> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11; };

template <class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M>
struct Tuple<A, B, C, D, E, F, G, H, I, J, K, L, M> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11; M _12; };

template <class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N>
struct Tuple<A, B, C, D, E, F, G, H, I, J, K, L, M, N> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11; M _12; N _13; };

template <class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O>
struct Tuple<A, B, C, D, E, F, G, H, I, J, K, L, M, N, O> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11; M _12; N _13; O _14; };

template <class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P>
struct Tuple<A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11; M _12; N _13; O _14; P _15; };
// clang-format on

}  // namespace detail

template <class... T>
struct Tuple : detail::Tuple<T...> {
  static constexpr u32 COUNT = sizeof...(T);
  using Inn = detail::Tuple<T...>;

 public:
  constexpr Tuple(T... args) noexcept : Inn{(T&&)(args)...} {}

 public:
  template <u32 I>
  auto operator[](trait::index_t<I> x) const -> auto& {
    return x[*this];
  }

  template <u32 I>
  auto operator[](trait::index_t<I> x) -> auto& {
    return x[*this];
  }

  auto operator==(const Tuple& other) const -> bool {
    auto accum = true;
    trait::for_each<0, COUNT>([&](auto I){
      accum = accum && (I[*this] == I[other]);
    });
    return accum;
  }

  void fmt(auto& f) const {
    auto imp = f.debug_tuple("");
    trait::for_each<0, COUNT>([&](auto I) { imp.field(I[*this]); });
    imp.finish();
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
