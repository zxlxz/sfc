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
// clang-format on

template <u32 I>
struct Tag {
  static constexpr u32 VALUE = I;

 public:
  auto operator[](auto&& inn) const noexcept -> auto& {
    if constexpr (I == 0) return inn._0;
    if constexpr (I == 1) return inn._1;
    if constexpr (I == 2) return inn._2;
    if constexpr (I == 3) return inn._3;
    if constexpr (I == 4) return inn._4;
    if constexpr (I == 5) return inn._5;
    if constexpr (I == 6) return inn._6;
    if constexpr (I == 7) return inn._7;
    if constexpr (I == 8) return inn._8;
    if constexpr (I == 9) return inn._9;
  }
};

template <class U, class T, class... S>
constexpr auto type_idx() -> u32 {
  if constexpr (sfc::same_<const U&, const T&>) {
    return 0;
  } else {
    static_assert(sizeof...(S) != 0);
    return 1 + type_idx<U, S...>();
  }
}

template <class... T>
class Variant {
  static_assert(sizeof...(T) <= 10, "Variant: only support up to 10 types");
  static constexpr u32 N = sizeof...(T);

  using Inn = Inner<T...>;

  u8 _tag;
  Inn _inn;

 public:
  template <u32 I, class... U>
  Variant(Tag<I> tag, U&&... u) : _tag{I}, _inn{} {
    ptr::write(&tag[_inn], static_cast<U&&>(u)...);
  }

  template <class U>
  Variant(U val) noexcept : Variant{Tag<type_idx<U, T...>()>{}, static_cast<U&&>(val)} {}

  ~Variant() {
    this->map_mut([&](auto& x) { mem::drop(x); });
  }

  Variant(Variant&& other) noexcept : _tag{other._tag} {
    this->imap([&](auto tag) { tag[_inn] = mem::move(tag[other._inn]); });
  }

  Variant(const Variant& other) noexcept : _tag{other._tag} {
    this->imap([&](auto tag) { tag[_inn] = tag[other._inn]; });
  }

  Variant& operator=(Variant&& other) noexcept {
    if (this != &other) {
      this->map_mut([&](auto& y) { mem::drop(y); });
      _tag = other._tag;
      this->imap([&](auto tag) { tag[_inn] = mem::move(tag[other._inn]); });
    }
    return *this;
  }

  Variant& operator=(const Variant& other) noexcept {
    if (this != &other) {
      this->map_mut([&](auto& y) { mem::drop(y); });
      _tag = other._tag;
      this->imap([&](auto tag) { tag[_inn] = tag[other._inn]; });
    }
    return *this;
  }

  template <class U>
  auto is() const noexcept -> bool {
    static constexpr auto IDX = type_idx<U, T...>();
    return _tag == IDX;
  }

  template <class U>
  auto as() const noexcept -> Option<const U&> {
    static constexpr auto tag = Tag<type_idx<U, T...>()>{};
    if (_tag != tag.VALUE) {
      return {};
    }
    return tag[_inn];
  }

  template <class U>
  auto as_mut() noexcept -> Option<U&> {
    static constexpr auto tag = Tag<type_idx<U, T...>()>{};
    if (_tag != tag.VALUE) {
      return {};
    }
    return tag[_inn];
  }

 public:
  void imap(auto&& f) const {
    switch (_tag) {
        // clang-format off
      case 0: if constexpr (N > 0) f(Tag<0>{}); break;
      case 1: if constexpr (N > 1) f(Tag<1>{}); break;
      case 2: if constexpr (N > 2) f(Tag<2>{}); break;
      case 3: if constexpr (N > 3) f(Tag<3>{}); break;
      case 4: if constexpr (N > 4) f(Tag<4>{}); break;
      case 5: if constexpr (N > 5) f(Tag<5>{}); break;
      case 6: if constexpr (N > 6) f(Tag<6>{}); break;
      case 7: if constexpr (N > 7) f(Tag<7>{}); break;
      case 8: if constexpr (N > 8) f(Tag<8>{}); break;
      case 9: if constexpr (N > 9) f(Tag<9>{}); break;
        // clang-format on
    }
  }

  void map(auto&& f) const {
    this->imap([&](auto tag) { f(tag[_inn]); });
  }

  void map_mut(auto&& f) {
    this->imap([&](auto tag) { f(tag[_inn]); });
  }

  void fmt(auto& f) const {
    this->imap([&](auto tag) { f.write_val(tag[_inn]); });
  }
};

}  // namespace sfc::variant

namespace sfc {
using variant::Variant;
}  // namespace sfc
