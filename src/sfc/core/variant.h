#pragma once

#include "sfc/core/expect.h"
#include "sfc/core/option.h"
#include "sfc/core/ptr.h"

namespace sfc::variant {

template <u32 I>
struct tag_t {};

template <class... T>
union Inner;

template <class T>
union Inner<T> {
  T _0;

 public:
  template <u32 I, class... U>
  Inner(tag_t<I>, U&&... u) noexcept : _0{static_cast<U&&>(u)...} {}

  Inner(u32 tag, Inner&& other) noexcept {
    if (tag == 0) {
      ptr::write(&_0, mem::move(other._0));
    }
  }

  Inner(u32 tag, const Inner& other) noexcept {
    if (tag == 0) {
      ptr::write(&_0, other._0);
    }
  }

  ~Inner() noexcept {}

  template <class U>
  static constexpr auto tag() -> u32 {
    return __is_same(T, U) ? 0U : 1U;
  }

  template <class U>
  auto as() const -> const U& {
    return _0;
  }

  template <class U>
  auto as_mut() -> U& {
    return _0;
  }

  auto map([[maybe_unused]] u32 tag, auto&& f) const {
    return f(_0);
  }

  auto map_mut([[maybe_unused]] u32 tag, auto&& f) {
    return f(_0);
  }
};

template <class T0, class... Ts>
union Inner<T0, Ts...> {
  T0 _0;
  Inner<Ts...> _1;

 public:
  template <class... U>
  Inner(tag_t<0>, U&&... u) : _0{static_cast<U&&>(u)...} {}

  template <u32 I, class... U>
  Inner(tag_t<I>, U&&... u) : _1{tag_t<I - 1>{}, static_cast<U&&>(u)...} {}

  Inner(u32 tag, Inner&& other) : _1{tag - 1, mem::move(other._1)} {
    if (tag == 0) {
      ptr::write(&_0, mem::move(other._0));
    }
  }

  Inner(u32 tag, const Inner& other) : _1{tag - 1, other._1} {
    if (tag == 0) {
      ptr::write(&_0, other._0);
    }
  }

  ~Inner() noexcept {}

  template <class U>
  static constexpr auto tag() -> u32 {
    if constexpr (__is_same(T0, U)) {
      return 0;
    } else {
      return Inner<Ts...>::template tag<U>() + 1;
    }
  }

  template <class U>
  auto as() const -> const U& {
    if constexpr (__is_same(T0, U)) {
      return _0;
    } else {
      return _1.template as<U>();
    }
  }

  template <class U>
  auto as_mut() -> U& {
    if constexpr (__is_same(T0, U)) {
      return _0;
    } else {
      return _1.template as_mut<U>();
    }
  }

  auto map(u32 tag, auto&& f) const {
    return tag == 0 ? f(_0) : _1.map(tag - 1, f);
  }

  auto map_mut(u32 tag, auto&& f) {
    return tag == 0 ? f(_0) : _1.map_mut(tag - 1, f);
  }
};

template <class... T>
class Variant {
  using Inn = Inner<T...>;

  u8 _tag;
  Inn _inn;

 public:
  template <u32 I, class... U>
  Variant(tag_t<I>, U&&... u) : _tag{I}, _inn{tag_t<I>{}, static_cast<U&&>(u)...} {
    static_assert(I < sizeof...(T), "variant::Variant: invalid tag");
  }

  template <class U, u32 I = Inn::template tag<U>()>
  Variant(U val) : _tag{I}, _inn{tag_t<I>{}, mem::move(val)} {
    static_assert(I < sizeof...(T), "variant::Variant: invalid type");
  }

  ~Variant() {
    _inn.map(_tag, [](auto& x) { mem::drop(x); });
  }

  Variant(Variant&& other) noexcept : _tag{other._tag}, _inn{_tag, mem::move(other._inn)} {}

  Variant(const Variant& other) : _tag{other._tag}, _inn{other._tag, other._inn} {}

  Variant& operator=(Variant&& other) noexcept {
    if (this != &other) {
      _inn.map_mut(_tag, [](auto& x) { mem::drop(x); });
      _inn.map_mut(other._tag, [&]<class U>(U& x) { ptr::write(&x, mem::move(other._inn.template as_mut<U>())); });
      _tag = other._tag;
    }
    return *this;
  }

  Variant& operator=(const Variant& other) {
    if (this == &other) {
      return *this;
    }
    _inn.map_mut(_tag, [](auto& x) { mem::drop(x); });
    _tag = other._tag;
    _inn = {other._tag, other._inn};
    return *this;
  }

  template <class U>
  auto is() const noexcept -> bool {
    return _tag == _inn.template tag<U>();
  }

  template <class U>
  auto as() const noexcept -> option::Option<const U&> {
    if (_tag != _inn.template tag<U>()) {
      return {};
    }
    return _inn.template as<U>();
  }

  template <class U>
  auto as_mut() noexcept -> Option<U&> {
    if (_tag != _inn.template tag<U>()) {
      return {};
    }
    return _inn.template as_mut<U>();
  }

  void map(auto&& f) const {
    return _inn.map(_tag, f);
  }

  void map_mut(auto&& f) {
    return _inn.map_mut(_tag, f);
  }

  void fmt(auto& f) const {
    _inn.map(_tag, [&](const auto& val) { fmt::Display::fmt(val, f); });
  }
};

}  // namespace sfc::variant

namespace sfc {
using variant::Variant;
}  // namespace sfc
