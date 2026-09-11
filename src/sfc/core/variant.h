#pragma once

#include "sfc/core/ptr.h"
#include "sfc/core/option.h"

namespace sfc::variant {

namespace detail {

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

template <u32 I>
struct tag_t {
  static constexpr u32 VALUE = I;

 public:
  static auto operator[](auto&& self) -> auto& {
    static_assert(I < 16, "Variant::at: index out of bounds");
    if constexpr (I == 0) return self._0;
    if constexpr (I == 1) return self._1;
    if constexpr (I == 2) return self._2;
    if constexpr (I == 3) return self._3;
    if constexpr (I == 4) return self._4;
    if constexpr (I == 5) return self._5;
    if constexpr (I == 6) return self._6;
    if constexpr (I == 7) return self._7;
    if constexpr (I == 8) return self._8;
    if constexpr (I == 9) return self._9;
    if constexpr (I == 10) return self._10;
    if constexpr (I == 11) return self._11;
    if constexpr (I == 12) return self._12;
    if constexpr (I == 13) return self._13;
    if constexpr (I == 14) return self._14;
    if constexpr (I == 15) return self._15;
  }
};

template <class U, class T, class... S>
consteval auto idx() {
  if constexpr (trait::same_<U, T>) {
    return 0;
  } else {
    return 1 + idx<U, S...>();
  }
}

template <class U, class... T>
consteval auto tag() {
  static constexpr u32 I = detail::idx<U, T...>();
  return tag_t<I>{};
}

template <u32 N>
static void imap(u32 I, auto&& f) {
  if constexpr (N > 0) I == 0 ? f(tag_t<0U>{}) : void();
  if constexpr (N > 1) I == 1 ? f(tag_t<1U>{}) : void();
  if constexpr (N > 2) I == 2 ? f(tag_t<2U>{}) : void();
  if constexpr (N > 3) I == 3 ? f(tag_t<3U>{}) : void();
  if constexpr (N > 4) I == 4 ? f(tag_t<4U>{}) : void();
  if constexpr (N > 5) I == 5 ? f(tag_t<5U>{}) : void();
  if constexpr (N > 6) I == 6 ? f(tag_t<6U>{}) : void();
  if constexpr (N > 7) I == 7 ? f(tag_t<7U>{}) : void();
  if constexpr (N > 8) I == 8 ? f(tag_t<8U>{}) : void();
  if constexpr (N > 9) I == 9 ? f(tag_t<9U>{}) : void();
  if constexpr (N > 10) I == 10 ? f(tag_t<10U>{}) : void();
  if constexpr (N > 11) I == 11 ? f(tag_t<11U>{}) : void();
  if constexpr (N > 12) I == 12 ? f(tag_t<12U>{}) : void();
  if constexpr (N > 13) I == 13 ? f(tag_t<13U>{}) : void();
  if constexpr (N > 14) I == 14 ? f(tag_t<14U>{}) : void();
  if constexpr (N > 15) I == 15 ? f(tag_t<15U>{}) : void();
}

}  // namespace detail

template <class... T>
class Variant {
  static constexpr u32 N = sizeof...(T);
  using Inn = detail::Union<T...>;
  u8 _tag;
  Inn _inn;

 public:
  template <trait::any_<T...> U>
  explicit Variant(U arg) noexcept {
    static constexpr auto I = detail::tag<U, T...>();
    ptr::write(&I[_inn], mem::move(arg));
    _tag = I.VALUE;
  }

  ~Variant() {
    detail::imap<N>(_tag, [&](auto I) { mem::drop(I[_inn]); });
    _tag = 0xFF;
  }

  Variant(Variant&& other) noexcept : _tag{other._tag} {
    detail::imap<N>(_tag, [&](auto I) { ptr::write(&I[_inn], mem::move(I[other._inn])); });
  }

  Variant& operator=(Variant&& other) noexcept {
    if (this != &other) {
      detail::imap<N>(_tag, [&](auto I) { mem::drop(I[_inn]); });
      _tag = other._tag;
      detail::imap<N>(_tag, [&](auto I) { ptr::write(&I[_inn], mem::move(I[other._inn])); });
    }
    return *this;
  }

 public:
  template <class U>
  auto is() const noexcept -> bool {
    static constexpr auto I = detail::tag<U, T...>();
    return _tag == I.VALUE;
  }

  template <class U>
  auto as() const noexcept -> Option<const U&> {
    static constexpr auto I = detail::tag<U, T...>();
    if (_tag != I.VALUE) return {};
    return I[_inn];
  }

  template <class U>
  auto as_mut() noexcept -> Option<U&> {
    static constexpr auto I = detail::tag<U, T...>();
    if (_tag != I.VALUE) return {};
    return I[_inn];
  }

 public:
  void map(auto&& f) const {
    detail::imap<N>(_tag, [&](auto I) { f(I[_inn]); });
  }

  void map_mut(auto&& f) {
    detail::imap<N>(_tag, [&](auto I) { f(I[_inn]); });
  }

  void fmt(auto& f) const {
    this->map([&](auto& val) { f.write_val(val); });
  }
};

}  // namespace sfc::variant

namespace sfc {
using variant::Variant;
}  // namespace sfc
