#pragma once

#include "sfc/core/ptr.h"
#include "sfc/core/option.h"

namespace sfc::variant {

template <class... T>
union _UnionData;

#define _UNION_IMPL(...)    \
  __VA_ARGS__;              \
  constexpr _UnionData() {} \
  constexpr ~_UnionData() {}

// clang-format off
template<>
union _UnionData<> { };

template <class A>
union _UnionData<A>{ _UNION_IMPL(A _0)};

template <class A, class B>
union _UnionData<A, B>{ _UNION_IMPL(A _0; B _1) };

template <class A, class B, class C>
union _UnionData<A, B, C> { _UNION_IMPL(A _0; B _1; C _2) };

template <class A, class B, class C, class D>
union _UnionData<A, B, C, D> { _UNION_IMPL(A _0; B _1; C _2; D _3) };

template <class A, class B, class C, class D, class E>
union _UnionData<A, B, C, D, E> { _UNION_IMPL(A _0; B _1; C _2; D _3; E _4) };

template <class A, class B, class C, class D, class E, class F>
union _UnionData<A, B, C, D, E, F> { _UNION_IMPL(A _0; B _1; C _2; D _3; E _4; F _5) };

template <class A, class B, class C, class D, class E, class F, class G>
union _UnionData<A, B, C, D, E, F, G> { _UNION_IMPL(A _0; B _1; C _2; D _3; E _4; F _5; G _6) };

template <class A, class B, class C, class D, class E, class F, class G, class H>
union _UnionData<A, B, C, D, E, F, G, H> { _UNION_IMPL(A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7) };

template <class A, class B, class C, class D, class E, class F, class G, class H, class I>
union _UnionData<A, B, C, D, E, F, G, H, I> { _UNION_IMPL(A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8) };

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J>
union _UnionData<A, B, C, D, E, F, G, H, I, J> { _UNION_IMPL(A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9) };

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K>
union _UnionData<A, B, C, D, E, F, G, H, I, J, K> { _UNION_IMPL(A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10) };

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L>
union _UnionData<A, B, C, D, E, F, G, H, I, J, K, L> { _UNION_IMPL(A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11) };

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M>
union _UnionData<A, B, C, D, E, F, G, H, I, J, K, L, M> { _UNION_IMPL(A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11; M _12) };

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N>
union _UnionData<A, B, C, D, E, F, G, H, I, J, K, L, M, N> { _UNION_IMPL(A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11; M _12; N _13) };

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O>
union _UnionData<A, B, C, D, E, F, G, H, I, J, K, L, M, N, O> { _UNION_IMPL(A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11; M _12; N _13; O _14) };

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P>
union _UnionData<A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P> { _UNION_IMPL(A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; I _8; J _9; K _10; L _11; M _12; N _13; O _14; P _15) };
// clang-format on

#undef _UNION_IMPL

template <class U, class... T>
consteval auto union_tag() -> u32 {
  static_assert(sizeof...(T) <= 16, "union_tag: too many types");

  const bool vals[] = {trait::same_<U, T>..., false};
  for (u32 i = 0; i < sizeof...(T); ++i) {
    if (vals[i]) return i;
  }
  return 0xFF;
}

template <u32 IDX>
auto union_at(auto&& u) -> auto& {
  static_assert(IDX < 16, "union_at: index out of bounds");

  if constexpr (IDX == 0) return u._0;
  if constexpr (IDX == 1) return u._1;
  if constexpr (IDX == 2) return u._2;
  if constexpr (IDX == 3) return u._3;
  if constexpr (IDX == 4) return u._4;
  if constexpr (IDX == 5) return u._5;
  if constexpr (IDX == 6) return u._6;
  if constexpr (IDX == 7) return u._7;
  if constexpr (IDX == 8) return u._8;
  if constexpr (IDX == 9) return u._9;
  if constexpr (IDX == 10) return u._10;
  if constexpr (IDX == 11) return u._11;
  if constexpr (IDX == 12) return u._12;
  if constexpr (IDX == 13) return u._13;
  if constexpr (IDX == 14) return u._14;
  if constexpr (IDX == 15) return u._15;
}

template <class... T>
class Variant {
  static constexpr u32 CNT = sizeof...(T);
  using Inn = _UnionData<T...>;
  u8 _tag;
  Inn _inn;

 public:
  template <class U>
  explicit Variant(U val) noexcept : _tag{0xFF} {
    static constexpr auto IDX = variant::union_tag<U, T...>();
    auto& element = variant::union_at<IDX>(_inn);
    ptr::write(&element, mem::move(val));
    _tag = IDX;
  }

  ~Variant() {
    this->map_mut([&](auto& v) { mem::drop(v); });
    _tag = 0xFF;
  }

  Variant(Variant&& other) noexcept : _tag{0xFF} {
    other.imap([&](auto I) {
      auto& dst = variant::union_at<I.VALUE>(_inn);
      auto& src = variant::union_at<I.VALUE>(other._inn);
      ptr::write(&dst, mem::move(src));
      _tag = I.VALUE;
    });
  }

  Variant(const Variant& other) noexcept : _tag{0xFF} {
    other.imap([&](auto I) {
      auto& dst = variant::union_at<I.VALUE>(_inn);
      auto& src = variant::union_at<I.VALUE>(other._inn);
      ptr::write(&dst, src);
      _tag = I.VALUE;
    });
  }

  Variant& operator=(Variant&& other) noexcept {
    if (this != &other) {
      this->map_mut([&](auto& v) { mem::drop(v); });
      other.imap([&](auto I) {
        auto& dst = variant::union_at<I.VALUE>(_inn);
        auto& src = variant::union_at<I.VALUE>(other._inn);
        ptr::write(&dst, mem::move(src));
        _tag = I.VALUE;
      });
    }
    return *this;
  }

  Variant& operator=(const Variant& other) noexcept {
    if (this != &other) {
      this->map_mut([&](auto& v) { mem::drop(v); });
      other.imap([&](auto I) {
        auto& dst = variant::union_at<I.VALUE>(_inn);
        auto& src = variant::union_at<I.VALUE>(other._inn);
        ptr::write(&dst, src);
        _tag = I.VALUE;
      });
    }
    return *this;
  }

  template <class U>
  auto is() const noexcept -> bool {
    static constexpr auto IDX = variant::union_tag<U, T...>();
    return _tag == IDX;
  }

  template <class U>
  auto as() const noexcept -> Option<const U&> {
    static constexpr auto IDX = variant::union_tag<U, T...>();
    if (_tag != IDX) return {};
    return variant::union_at<IDX>(_inn);
  }

  template <class U>
  auto as_mut() noexcept -> Option<U&> {
    static constexpr auto IDX = variant::union_tag<U, T...>();
    if (_tag != IDX) return {};
    return variant::union_at<IDX>(_inn);
  }

  void map(auto&& f) const {
    this->imap([&](auto I) { f(variant::union_at<I.VALUE>(_inn)); });
  }

  void map_mut(auto&& f) {
    this->imap([&](auto I) { f(variant::union_at<I.VALUE>(_inn)); });
  }

  void imap(auto&& f) const {
    if constexpr (CNT > 0) _tag == 0 ? f(trait::const_t<0U>{}) : void();
    if constexpr (CNT > 1) _tag == 1 ? f(trait::const_t<1U>{}) : void();
    if constexpr (CNT > 2) _tag == 2 ? f(trait::const_t<2U>{}) : void();
    if constexpr (CNT > 3) _tag == 3 ? f(trait::const_t<3U>{}) : void();
    if constexpr (CNT > 4) _tag == 4 ? f(trait::const_t<4U>{}) : void();
    if constexpr (CNT > 5) _tag == 5 ? f(trait::const_t<5U>{}) : void();
    if constexpr (CNT > 6) _tag == 6 ? f(trait::const_t<6U>{}) : void();
    if constexpr (CNT > 7) _tag == 7 ? f(trait::const_t<7U>{}) : void();
    if constexpr (CNT > 8) _tag == 8 ? f(trait::const_t<8U>{}) : void();
    if constexpr (CNT > 9) _tag == 9 ? f(trait::const_t<9U>{}) : void();
    if constexpr (CNT > 10) _tag == 10 ? f(trait::const_t<10U>{}) : void();
    if constexpr (CNT > 11) _tag == 11 ? f(trait::const_t<11U>{}) : void();
    if constexpr (CNT > 12) _tag == 12 ? f(trait::const_t<12U>{}) : void();
    if constexpr (CNT > 13) _tag == 13 ? f(trait::const_t<13U>{}) : void();
    if constexpr (CNT > 14) _tag == 14 ? f(trait::const_t<14U>{}) : void();
    if constexpr (CNT > 15) _tag == 15 ? f(trait::const_t<15U>{}) : void();
  }

 public:
  void fmt(auto& f) const {
    this->map([&](const auto& t) { f.write_val(t); });
  }
};

}  // namespace sfc::variant

namespace sfc {
using variant::Variant;
}  // namespace sfc
