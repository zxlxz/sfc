#pragma once

#include "sfc/core/mod.h"

namespace sfc::variant {

template <u32 I>
struct tag_t {};

template <class... T>
union Union;

template <>
union Union<> {};

template <class T>
union Union<T> {
  T _0;

 public:
  template <u32 I, class... U>
  Union(tag_t<I>, U&&... u) : _0{static_cast<U&&>(u)...} {}

  Union(u32 tag, Union&& other) {
    if (tag == 0) {
      ptr::write(&_0, static_cast<T&&>(other._0));
    }
  }

  Union(u32 tag, const Union& other) {
    if (tag == 0) {
      ptr::write(&_0, other._0);
    }
  }

  ~Union() {}

  template <class U>
  static constexpr auto tag() -> u32 {
    return __is_same(T, U) ? 0U : 1U;
  }

  operator const T&() const {
    return _0;
  }

  operator T&() {
    return _0;
  }

  operator T&&() {
    return static_cast<T&&>(_0);
  }

  auto map([[maybe_unused]] u32 tag, auto&& f) const {
    return f(_0);
  }

  auto map_mut([[maybe_unused]] u32 tag, auto&& f) {
    return f(_0);
  }
};

template <class T0, class... Ts>
union Union<T0, Ts...> {
  T0 _0;
  Union<Ts...> _1;

 public:
  template <class... U>
  Union(tag_t<0>, U&&... u) : _0{static_cast<U&&>(u)...} {}

  template <u32 I, class... U>
  Union(tag_t<I>, U&&... u) : _1{tag_t<I - 1>{}, static_cast<U&&>(u)...} {}

  Union(u32 tag, Union&& other) : _1{tag - 1, static_cast<Union<Ts...>&&>(other._1)} {
    if (tag == 0) {
      new (&_0) T0{static_cast<T0&&>(other._0)};
    }
  }

  Union(u32 tag, const Union& other) : _1{tag - 1, other._1} {
    if (tag == 0) {
      new (&_0) T0{other._0};
    }
  }

  ~Union() {}

  template <class U>
  static constexpr auto tag() -> u32 {
    if constexpr (__is_same(T0, U)) {
      return 0;
    } else {
      return Union<Ts...>::template tag<U>() + 1;
    }
  }

  template <class U>
  operator const U&() const {
    if constexpr (__is_same(T0, U)) {
      return _0;
    } else {
      return static_cast<const U&>(_1);
    }
  }

  template <class U>
  operator U&() {
    if constexpr (__is_same(T0, U)) {
      return _0;
    } else {
      return static_cast<U&>(_1);
    }
  }

  template <class U>
  operator U&&() {
    if constexpr (__is_same(T0, U)) {
      return static_cast<U&&>(_0);
    } else {
      return static_cast<U&&>(_1);
    }
  }

  auto map(u32 tag, auto&& f) const {
    if (tag == 0) {
      return f(_0);
    } else {
      return _1.map(tag - 1, f);
    }
  }

  auto map_mut(u32 tag, auto&& f) {
    if (tag == 0) {
      return f(_0);
    } else {
      return _1.map_mut(tag - 1, f);
    }
  }
};

template <class... T>
class Variant {
  using Inn = Union<T...>;

  u8 _tag;
  Inn _inn;

 public:
  template <u32 I, class... U>
  Variant(tag_t<I>, U&&... u) : _tag{I}, _inn{tag_t<I>{}, static_cast<U&&>(u)...} {
    static_assert(I < sizeof...(T), "variant::Variant: invalid tag");
  }

  template <class U, u32 I = Inn::template tag<U>()>
  Variant(U val) : _tag{I}, _inn{tag_t<I>{}, static_cast<U&&>(val)} {
    static_assert(I < sizeof...(T), "variant::Variant: invalid type");
  }

  ~Variant() {
    _inn.map(_tag, []<class U>(U& x) { x.~U(); });
  }

  Variant(Variant&& other) noexcept
      : _tag{other._tag}, _inn{_tag, static_cast<Inn&&>(other._inn)} {}

  Variant(const Variant& other) : _tag{other._tag}, _inn{other._tag, other._inn} {}

  Variant& operator=(Variant&& other) noexcept {
    if (this != &other) {
      _inn.map_mut(_tag, []<class U>(U& x) { x.~U(); });
      _tag = other._tag;
      _inn.map_mut(_tag, [&]<class U>(U& x) { ptr::write(&x, static_cast<U&&>(other._inn)); });
    }
    return *this;
  }

  Variant& operator=(const Variant& other) {
    if (this != &other) {
      _inn.map_mut(_tag, []<class U>(U& x) { x.~U(); });
      _tag = other._tag;
      _inn = {other._tag, other._inn};
    }
    return *this;
  }

  template <class U>
  auto is() const -> bool {
    return _tag == _inn.template tag<U>();
  }

  template <class U>
  auto as() const -> const U& {
    panicking::assert(_tag == _inn.template tag<U>(), "variant::Variant::as: invalid type");
    return static_cast<const U&>(_inn);
  }

  template <class U>
  auto as_mut() -> U& {
    panicking::assert(_tag == _inn.template tag<U>(), "variant::Variant::as_mut: invalid type");
    return static_cast<U&>(_inn);
  }

  void map(auto&& f) const {
    return _inn.map(_tag, f);
  }

  void map_mut(auto&& f) {
    return _inn.map_mut(_tag, f);
  }
};

}  // namespace sfc::variant
