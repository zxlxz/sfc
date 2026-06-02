#pragma once

#include "sfc/core/ptr.h"
#include "sfc/core/option.h"

namespace sfc::variant {

namespace impl {
template <class... T>
union Storage;

#define _VF              \
  constexpr Storage() {} \
  constexpr ~Storage() {}

// clang-format off
template<>
union Storage<> { };

template <class A>
union Storage<A>{A _0;_VF};

template <class A, class B>
union Storage<A,B>{A _0; B _1; _VF};

template <class A, class B, class C>
union Storage<A,B,C> { A _0; B _1; C _2; _VF};

template <class A, class B, class C, class D>
union Storage<A,B,C,D> { A _0; B _1; C _2; D _3; _VF};

template <class A, class B, class C, class D, class E>
union Storage<A,B,C,D,E> { A _0; B _1; C _2; D _3; E _4; _VF};

template <class A, class B, class C, class D, class E, class F>
union Storage<A,B,C,D,E,F> { A _0; B _1; C _2; D _3; E _4; F _5; _VF};

template <class A, class B, class C, class D, class E, class F, class G>
union Storage<A,B,C,D,E,F,G> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; _VF};

template <class A, class B, class C, class D, class E, class F, class G, class H>
union Storage<A,B,C,D,E,F,G,H> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; _VF};

template <class A, class B, class C, class D, class E, class F, class G, class H, class I>
union Storage<A,B,C,D,E,F,G,H,I> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; _VF};

template <class A, class B, class C, class D, class E, class F, class G, class H, class I, class J>
union Storage<A,B,C,D,E,F,G,H,I,J> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; _VF};

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K>
union Storage<A,B,C,D,E,F,G,H,I,J,K> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; _VF};

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L>
union Storage<A,B,C,D,E,F,G,H,I,J,K,L> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11; _VF};

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M>
union Storage<A,B,C,D,E,F,G,H,I,J,K,L,M> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11; M _12; _VF};

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N>
union Storage<A,B,C,D,E,F,G,H,I,J,K,L,M,N> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11; M _12; N _13; _VF};

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O>
union Storage<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11; M _12; N _13; O _14; _VF};

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P>
union Storage<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9 ; K _10; L _11; M _12; N _13; O _14; P _15; _VF};
// clang-format on
}  // namespace impl

template <u32 I>
struct idx_t {
  static constexpr auto VALUE = I;

  [[gnu::always_inline]] static auto operator[](auto& t) -> auto& {
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

template <class... T>
class Variant {
  static constexpr u32 N = sizeof...(T);
  using Inn = impl::Storage<T...>;
  u8 _tag;
  Inn _inn;

 public:
  template <class U>
  static consteval auto tag_of() -> u32 {
    static constexpr bool matchs[] = {trait::same_<U, T>..., false};
    for (u32 i = 0; i < sizeof...(T); ++i) {
      if (matchs[i]) return i;
    }
    return sizeof...(T);
  }

  template <u32 I, class U>
  Variant(idx_t<I> tag, U val) : _tag{I}, _inn{} {
    ptr::write(&tag[_inn], mem::move(val));
  }

  template <trait::any_<T...> U, u32 I = Variant::tag_of<U>()>
  explicit Variant(U val) noexcept : Variant{idx_t<I>{}, mem::move(val)} {}

  ~Variant() {
    this->map_mut([&](auto& v) { mem::drop(v); });
  }

  Variant(Variant&& other) noexcept : _tag{0xFF} {
    other.imap([&](auto I) {
      ptr::write(&I[_inn], mem::move(I[other._inn]));
      _tag = other._tag;
    });
  }

  Variant(const Variant& other) noexcept : _tag{0xFF} {
    other.imap([&](auto I) {
      ptr::write(&I[_inn], I[other._inn]);
      _tag = other._tag;
    });
  }

  Variant& operator=(Variant&& other) noexcept {
    if (this == &other) return *this;
    this->map_mut([&](auto& v) {
      mem::drop(v);
      _tag = 0xFF;
    });
    other.imap([&](auto I) {
      ptr::write(&I[_inn], mem::move(I[other._inn]));
      _tag = other._tag;
    });
    return *this;
  }

  Variant& operator=(const Variant& other) noexcept {
    if (this == &other) return *this;
    this->map_mut([&](auto& v) {
      mem::drop(v);
      _tag = 0xFF;
    });
    other.imap([&](auto I) {
      ptr::write(&I[_inn], I[other._inn]);
      _tag = other._tag;
    });
    return *this;
  }

  template <class U>
  auto is() const noexcept -> bool {
    static constexpr auto IDX = Variant::tag_of<U>();
    return _tag == IDX;
  }

  template <class U>
  auto as() const noexcept -> Option<const U&> {
    static constexpr auto IDX = Variant::tag_of<U>();
    if (_tag != IDX) return {};
    return idx_t<IDX>::operator[](_inn);
  }

  template <class U>
  auto as_mut() noexcept -> Option<U&> {
    static constexpr auto IDX = Variant::tag_of<U>();
    if (_tag != IDX) return {};
    return idx_t<IDX>::operator[](_inn);
  }

  void imap(auto&& f) const {
    // clang-format off
    switch (_tag) {
      case 0:  if constexpr (N > 0)   return f(idx_t<0>{});
      case 1:  if constexpr (N > 1)   return f(idx_t<1>{});
      case 2:  if constexpr (N > 2)   return f(idx_t<2>{});
      case 3:  if constexpr (N > 3)   return f(idx_t<3>{});
      case 4:  if constexpr (N > 4)   return f(idx_t<4>{});
      case 5:  if constexpr (N > 5)   return f(idx_t<5>{});
      case 6:  if constexpr (N > 6)   return f(idx_t<6>{});
      case 7:  if constexpr (N > 7)   return f(idx_t<7>{});
      case 8:  if constexpr (N > 8)   return f(idx_t<8>{});
      case 9:  if constexpr (N > 9)   return f(idx_t<9>{});
      case 10: if constexpr (N > 10)  return f(idx_t<10>{});
      case 11: if constexpr (N > 11)  return f(idx_t<11>{});
      case 12: if constexpr (N > 12)  return f(idx_t<12>{});
      case 13: if constexpr (N > 13)  return f(idx_t<13>{});
      case 14: if constexpr (N > 14)  return f(idx_t<14>{});
      case 15: if constexpr (N > 15)  return f(idx_t<15>{});
      default: break; // for 0xFF
    }
    // clang-format on
  }

  void map(auto&& f) const {
    this->imap([&](auto I) { f(I[_inn]); });
  }

  void map_mut(auto&& f) {
    this->imap([&](auto I) { f(I[_inn]); });
  }

 public:
  void fmt(auto& f) const {
    this->imap([&](auto I) { f.write_val(I[_inn]); });
  }
};

}  // namespace sfc::variant

namespace sfc {
using variant::Variant;
}  // namespace sfc
