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
// clang-format on

#undef _UNION_IMPL

template <class U, class... T>
consteval auto union_tag() -> u32 {
  static_assert(sizeof...(T) <= 8, "union_tag: too many types");

  const bool vals[] = {trait::same_<U, T>..., false};
  for (u32 i = 0; i < sizeof...(T); ++i) {
    if (vals[i]) return i;
  }
  return 0xFF;
}

template <u32 IDX>
auto union_at(auto&& u) -> decltype(auto) {
  static_assert(IDX <= 8, "union_at: index out of bounds");

  if constexpr (IDX == 0) return u._0;
  if constexpr (IDX == 1) return u._1;
  if constexpr (IDX == 2) return u._2;
  if constexpr (IDX == 3) return u._3;
  if constexpr (IDX == 4) return u._4;
  if constexpr (IDX == 5) return u._5;
  if constexpr (IDX == 6) return u._6;
  if constexpr (IDX == 7) return u._7;
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
  }

 public:
  void fmt(auto& f) const {
    this->map([&](const auto& v) { f.write_val(v); });
  }
};

}  // namespace sfc::variant

namespace sfc {
using variant::Variant;
}  // namespace sfc
