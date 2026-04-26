#pragma once

#include "sfc/core/option.h"
#include "sfc/core/ptr.h"

namespace sfc::variant {

template <class... T>
union Inner;

// clang-format off
template<>
union Inner<> { };

template <class A>
union Inner<A>{ A _0; };

template <class A, class B>
union Inner<A,B>{ A _0; B _1;};

template <class A, class B, class C>
union Inner<A,B,C> { A _0; B _1; C _2; };

template <class A, class B, class C, class D>
union Inner<A,B,C,D> { A _0; B _1; C _2; D _3; };

template <class A, class B, class C, class D, class E>
union Inner<A,B,C,D,E> { A _0; B _1; C _2; D _3; E _4; };

template <class A, class B, class C, class D, class E, class F>
union Inner<A,B,C,D,E,F> { A _0; B _1; C _2; D _3; E _4; F _5; };

template <class A, class B, class C, class D, class E, class F, class G>
union Inner<A,B,C,D,E,F,G> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; };

template <class A, class B, class C, class D, class E, class F, class G, class H>
union Inner<A,B,C,D,E,F,G,H> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; };

template <class A, class B, class C, class D, class E, class F, class G, class H, class I>
union Inner<A,B,C,D,E,F,G,H,I> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; };

template <class A, class B, class C, class D, class E, class F, class G, class H, class I, class J>
union Inner<A,B,C,D,E,F,G,H,I,J> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; };

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K>
union Inner<A,B,C,D,E,F,G,H,I,J,K> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; };

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L>
union Inner<A,B,C,D,E,F,G,H,I,J,K,L> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11; };

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M>
union Inner<A,B,C,D,E,F,G,H,I,J,K,L,M> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11; M _12; };

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N>
union Inner<A,B,C,D,E,F,G,H,I,J,K,L,M,N> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11; M _12; N _13; };

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O>
union Inner<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11; M _12; N _13; O _14; };

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P>
union Inner<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9 ; K _10; L _11; M _12; N _13; O _14; P _15; };
// clang-format on

template <class U, class... T>
struct _TypeIndexOf;

template <class U, class... S>
struct _TypeIndexOf<U, U, S...> {
  static constexpr auto VALUE = 0U;
};

template <class U, class T, class... S>
struct _TypeIndexOf<U, T, S...> {
  static constexpr auto VALUE = 1U + _TypeIndexOf<U, S...>::VALUE;
};

template <class U, class... T>
using idx_t = tuple::idx_t<_TypeIndexOf<U, T...>::VALUE>;

struct Tag {
  u8 _idx;

  template <u32 N>
  void map(auto& t, auto&& f) const {
    // clang-format off
    switch (_idx) {
      case 0: if constexpr  (N > 0)   return f(t._0);
      case 1: if constexpr  (N > 1)   return f(t._1);
      case 2: if constexpr  (N > 2)   return f(t._2);
      case 3: if constexpr  (N > 3)   return f(t._3);
      case 4: if constexpr  (N > 4)   return f(t._4);
      case 5: if constexpr  (N > 5)   return f(t._5);
      case 6: if constexpr  (N > 6)   return f(t._6);
      case 7: if constexpr  (N > 7)   return f(t._7);
      case 8: if constexpr  (N > 8)   return f(t._8);
      case 9: if constexpr  (N > 9)   return f(t._9);
      case 10: if constexpr (N > 10)  return f(t._10);
      case 11: if constexpr (N > 11)  return f(t._11);
      case 12: if constexpr (N > 12)  return f(t._12);
      case 13: if constexpr (N > 13)  return f(t._13);
      case 14: if constexpr (N > 14)  return f(t._14);
      case 15: if constexpr (N > 15)  return f(t._15);
    }
    // clang-format on
  }

  template <u32 N>
  void imap(auto&& f) const {
    // clang-format off
    switch (_idx) {
      case 0: if constexpr  (N > 0)   return f(tuple::idx_t<0>{});
      case 1: if constexpr  (N > 1)   return f(tuple::idx_t<1>{});
      case 2: if constexpr  (N > 2)   return f(tuple::idx_t<2>{});
      case 3: if constexpr  (N > 3)   return f(tuple::idx_t<3>{});
      case 4: if constexpr  (N > 4)   return f(tuple::idx_t<4>{});
      case 5: if constexpr  (N > 5)   return f(tuple::idx_t<5>{});
      case 6: if constexpr  (N > 6)   return f(tuple::idx_t<6>{});
      case 7: if constexpr  (N > 7)   return f(tuple::idx_t<7>{});
      case 8: if constexpr  (N > 8)   return f(tuple::idx_t<8>{});
      case 9: if constexpr  (N > 9)   return f(tuple::idx_t<9>{});
      case 10: if constexpr (N > 10)  return f(tuple::idx_t<10>{});
      case 11: if constexpr (N > 11)  return f(tuple::idx_t<11>{});
      case 12: if constexpr (N > 12)  return f(tuple::idx_t<12>{});
      case 13: if constexpr (N > 13)  return f(tuple::idx_t<13>{});
      case 14: if constexpr (N > 14)  return f(tuple::idx_t<14>{});
      case 15: if constexpr (N > 15)  return f(tuple::idx_t<15>{});
    }
    // clang-format on
  }
};

template <class... T>
class Variant {
  static constexpr u32 N = sizeof...(T);
  using Inn = Inner<T...>;

  Tag _tag;
  Inn _inn;

 public:
  template <u32 I, class U>
  Variant(tuple::idx_t<I> tag, U val) : _tag{I}, _inn{} {
    ptr::write(&tag[_inn], static_cast<U&&>(val));
  }

  template <class U>
  Variant(U val) noexcept : Variant{idx_t<U, T...>{}, static_cast<U&&>(val)} {}

  ~Variant() {
    this->map_mut([](auto& x) { mem::drop(x); });
  }

  Variant(Variant&& other) noexcept : _tag{other._tag} {
    this->imap_mut([&](auto I, auto& t) { ptr::write(&t, mem::move(I[other._inn])); });
  }

  Variant(const Variant& other) noexcept : _tag{other._tag} {
    this->imap_mut([&](auto I, auto& t) { ptr::write(&t, I[other._inn]); });
  }

  Variant& operator=(Variant&& other) noexcept {
    if (this != &other) {
      this->map_mut([](auto& x) { mem::drop(x); });
      _tag = other._tag;
      this->imap_mut([&](auto I, auto& t) { ptr::write(&t, mem::move(I[other._inn])); });
    }
    return *this;
  }

  Variant& operator=(const Variant& other) noexcept {
    if (this != &other) {
      this->map_mut([](auto& x) { mem::drop(x); });
      _tag = other._tag;
      this->imap_mut([&](auto I, auto& t) { ptr::write(&t, I[other._inn]); });
    }
    return *this;
  }

  template <class U>
  auto is() const noexcept -> bool {
    static constexpr auto TAG = idx_t<U, T...>{};
    return _tag._idx == TAG.VALUE;
  }

  template <class U>
  auto as() const noexcept -> Option<const U&> {
    static constexpr auto TAG = idx_t<U, T...>{};
    if (_tag._idx != TAG.VALUE) {
      return {};
    }
    return TAG[_inn];
  }

  template <class U>
  auto as_mut() noexcept -> Option<U&> {
    static constexpr auto TAG = idx_t<U, T...>{};
    if (_tag._idx != TAG.VALUE) {
      return {};
    }
    return TAG[_inn];
  }

 public:
  void map(auto&& f) const {
    _tag.map<N>(_inn, [&](auto& x) { f(x); });
  }

  void map_mut(auto&& f) {
    _tag.map<N>(_inn, [&](auto& x) { f(x); });
  }

  void imap_mut(auto&& f) {
    _tag.imap<N>([&](auto I) { f(I, I[_inn]); });
  }

  void fmt(auto& f) const {
    _tag.map<N>(_inn, [&](auto& x) { f.write_val(x); });
  }
};

}  // namespace sfc::variant

namespace sfc {
using variant::Variant;
}  // namespace sfc
