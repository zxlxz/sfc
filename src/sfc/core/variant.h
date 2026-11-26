#pragma once

#include "sfc/core/ptr.h"
#include "sfc/core/option.h"

namespace sfc::variant {

template <class... T>
struct Union;

// clang-format off
#define _UNION_FUNCS  \
  constexpr Union() {}  \
  constexpr ~Union() {}

template<>
struct Union<> { };

template <class A>
struct Union<A>{ union{A _0;}; _UNION_FUNCS };

template <class A, class B>
struct Union<A, B>{ union{A _0; B _1;}; _UNION_FUNCS };

template <class A, class B, class C>
struct Union<A, B, C> { union{A _0; B _1; C _2;}; _UNION_FUNCS };

template <class A, class B, class C, class D>
struct Union<A, B, C, D> { union{A _0; B _1; C _2; D _3;}; _UNION_FUNCS };

template <class A, class B, class C, class D, class E>
struct Union<A, B, C, D, E> { union{A _0; B _1; C _2; D _3; E _4;}; _UNION_FUNCS };

template <class A, class B, class C, class D, class E, class F>
struct Union<A, B, C, D, E, F> { union{A _0; B _1; C _2; D _3; E _4; F _5;}; _UNION_FUNCS };

template <class A, class B, class C, class D, class E, class F, class G>
struct Union<A, B, C, D, E, F, G> { union{A _0; B _1; C _2; D _3; E _4; F _5; G _6;}; _UNION_FUNCS };

template <class A, class B, class C, class D, class E, class F, class G, class H>
struct Union<A, B, C, D, E, F, G, H> { union{A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7;}; _UNION_FUNCS };

template <class A, class B, class C, class D, class E, class F, class G, class H, class I>
struct Union<A, B, C, D, E, F, G, H, I> { union{A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8;}; _UNION_FUNCS };

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J>
struct Union<A, B, C, D, E, F, G, H, I, J> { union{A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9;}; _UNION_FUNCS };

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K>
struct Union<A, B, C, D, E, F, G, H, I, J, K> { union{A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10;}; _UNION_FUNCS };

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L>
struct Union<A, B, C, D, E, F, G, H, I, J, K, L> { union{A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11;}; _UNION_FUNCS };

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M>
struct Union<A, B, C, D, E, F, G, H, I, J, K, L, M> { union{A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11; M _12;}; _UNION_FUNCS };

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N>
struct Union<A, B, C, D, E, F, G, H, I, J, K, L, M, N> { union{A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11; M _12; N _13;}; _UNION_FUNCS };

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O>
struct Union<A, B, C, D, E, F, G, H, I, J, K, L, M, N, O> { union{A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11; M _12; N _13; O _14;}; _UNION_FUNCS };

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P>
struct Union<A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P> { union{A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11; M _12; N _13; O _14; P _15;}; _UNION_FUNCS };
// clang-format on

#undef _UNION_IMPL

template <class... T>
class Variant {
  static constexpr u32 N = sizeof...(T);
  using Inn = Union<T...>;

  u8 _tag;
  Inn _inn;

 public:
  template <trait::any_<T...> U>
  explicit Variant(U arg) noexcept {
    constexpr auto I = type_index<U>();
    ptr::write(&I[_inn], mem::move(arg));
    _tag = u8(I.VALUE);
  }

  ~Variant() {
    this->map_mut([&](auto& x) { mem::drop(x); });
    _tag = 0xFF;
  }

  Variant(Variant&& other) noexcept : _tag{other._tag} {
    this->imap([&](auto I) { ptr::write(&I[_inn], mem::move(I[other._inn])); });
  }

  Variant& operator=(Variant&& other) noexcept {
    if (this != &other) {
      this->map_mut([&](auto& x) { mem::drop(x); });
      _tag = other._tag;
      this->imap([&](auto I) { ptr::write(&I[_inn], mem::move(I[other._inn])); });
    }
    return *this;
  }

 public:
  template <class U>
  auto is() const noexcept -> bool {
    constexpr auto I = type_index<U>();
    return _tag == I.VALUE;
  }

  template <class U>
  auto as() const noexcept -> Option<const U&> {
    constexpr auto I = type_index<U>();
    if (_tag != I.VALUE) return {};
    return I[_inn];
  }

  template <class U>
  auto as_mut() noexcept -> Option<U&> {
    constexpr auto I = type_index<U>();
    if (_tag != I.VALUE) return {};
    return I[_inn];
  }

 public:
  void map(auto&& f) const {
    this->imap([&](auto I) { f(I[_inn]); });
  }

  void map_mut(auto&& f) {
    this->imap([&](auto I) { f(I[_inn]); });
  }

  void fmt(auto& f) const {
    this->map([&](const auto& x) { f.write_val(x); });
  }

 private:
  template <class U>
  static consteval auto type_index() {
    static_assert(trait::any_<U, T...>, "Type U must be one of the variant types");
    constexpr auto f = []() {
      u32 inc = 1;
      u32 idx = 0;
      ((trait::same_<U, T> ? inc = 0 : idx += inc), ...);
      return idx;
    };
    return trait::index_t<f()>();
  }

  void imap(auto&& f) const {
#define IMAP_CASE(I) \
  if constexpr (I < N) f(trait::index_t<I>{});
    switch (_tag) {
      case 0:  IMAP_CASE(0); break;
      case 1:  IMAP_CASE(1); break;
      case 2:  IMAP_CASE(2); break;
      case 3:  IMAP_CASE(3); break;
      case 4:  IMAP_CASE(4); break;
      case 5:  IMAP_CASE(5); break;
      case 6:  IMAP_CASE(6); break;
      case 7:  IMAP_CASE(7); break;
      case 8:  IMAP_CASE(8); break;
      case 9:  IMAP_CASE(9); break;
      case 10: IMAP_CASE(10); break;
      case 11: IMAP_CASE(11); break;
      case 12: IMAP_CASE(12); break;
      case 13: IMAP_CASE(13); break;
      case 14: IMAP_CASE(14); break;
      case 15: IMAP_CASE(15); break;
    }
#undef IMAP_CASE
  }
};

}  // namespace sfc::variant

namespace sfc {
using variant::Variant;
}  // namespace sfc
