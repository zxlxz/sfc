#pragma once

#include "tuple.h"

namespace sfc::variant {

using tuple::idx_t;

namespace detail {

template <class... T>
union Union;

template <>
union Union<> {
  [[sfc_inline]] Union() {}
  [[sfc_inline]] ~Union() {}

  [[sfc_inline]] Union(idx_t<0>, auto&&...) noexcept {}

  template <usize>
  [[sfc_inline]] void get_unchecked() const {}

  template <usize>
  [[sfc_inline]] void get_unchecked_mut() {}

  [[sfc_inline]] void drop(usize) {}

  [[sfc_inline]] void move_ctor(usize, Union&) {}
  [[sfc_inline]] void copy_ctor(usize, const Union&) {}

  [[sfc_inline]] void move_assign(usize, Union&) {}
  [[sfc_inline]] void copy_assign(usize, const Union&) {}

  [[sfc_inline]] void map(usize, auto&&) const {}

  [[sfc_inline]] void map_mut(usize, auto&&) {}
};

template <class T0, class... Ts>
union Union<T0, Ts...> {
  T0 _0;
  Union<Ts...> _1;

 public:
  [[sfc_inline]] Union() {}

  [[sfc_inline]] ~Union() {}

  template <class... U>
  [[sfc_inline]] Union(idx_t<0>, U&&... args) noexcept : _0{static_cast<U&&>(args)...} {}

  template <usize I, class... U>
  [[sfc_inline]] Union(idx_t<I>, U&&... args) noexcept
      : _1{idx_t<I - 1>{}, static_cast<U&&>(args)...} {}

  template <trait::Same<T0> U>
  static constexpr auto tag() -> u32 {
    return 0;
  }

  template <class U>
  static constexpr auto tag() -> u32 {
    return Union<Ts...>::template tag<U>() + 1;
  }

  template <usize I>
    requires(I == 0)
  auto get_unchecked() const -> const auto& {
    return _0;
  }

  template <usize I>
  [[sfc_inline]] auto get_unchecked() const -> const auto& {
    return _1.template get_unchecked<I - 1>();
  }

  template <usize I>
    requires(I == 0)
  auto get_unchecked_mut() -> auto& {
    return _0;
  }

  template <usize I>
  [[sfc_inline]] auto get_unchecked_mut() -> auto& {
    return _1.template get_unchecked_mut<I - 1>();
  }

  [[sfc_inline]] void drop(usize idx) {
    if (idx == 0) {
      mem::drop(_0);
    } else {
      _1.drop(idx - 1);
    }
  }

  [[sfc_inline]] void move_ctor(usize idx, Union& src) {
    if (idx == 0) {
      ptr::write(&_0, static_cast<T0&&>(src._0));
    } else {
      _1.move_ctor(idx - 1, src._1);
    }
  }

  [[sfc_inline]] void copy_ctor(usize idx, Union& src) {
    if (idx == 0) {
      ptr::write(&_0, src._0);
    } else {
      _1.copy_ctor(idx - 1, src._1);
    }
  }

  [[sfc_inline]] void move_assign(usize idx, Union& src) {
    if (idx == 0) {
      _0 = static_cast<T0&&>(src._0);
    } else {
      _1.move_assign(idx - 1, src._1);
    }
  }

  [[sfc_inline]] void copy_assign(usize idx, const Union& src) {
    if (idx == 0) {
      _0 = src._0;
    } else {
      _1.copy_assign(idx - 1, src._1);
    }
  }

  void map(usize idx, auto&& f) const {
    if (idx == 0) {
      f(_0);
    } else {
      _1.map(idx - 1, f);
    }
  }

  void map_mut(usize idx, auto&& f) {
    if (idx == 0) {
      f(_0);
    } else {
      _1.map_mut(idx - 1, f);
    }
  }
};

}  // namespace detail

template <class... T>
class Variant {
  using Tag = u8;
  using Imp = detail::Union<T...>;

  template <class U>
  using tag_t = idx_t<Imp::template tag<U>()>;

  Tag _tag;
  Imp _imp;

 public:
  template <class U>
  explicit Variant(U val) noexcept
      : _tag{static_cast<Tag>(tag_t<U>::VALUE)}, _imp{tag_t<U>{}, static_cast<U&&>(val)} {}

  ~Variant() {
    _imp.drop(_tag);
  }

  Variant(const Variant& other) noexcept : _tag{other._tag} {
    _imp.copy_ctor(_tag, other._imp);
  }

  Variant(Variant&& other) noexcept : _tag{other._tag} {
    _imp.move_ctor(_tag, other._imp);
  }

  Variant& operator=(Variant&& other) noexcept {
    if (_tag == other._tag) {
      _imp.move_assign(_tag, other._imp);
    } else {
      _imp.drop(_tag);
      _imp.move_ctor(other._tag, other._imp);
      _tag = other._tag;
    }
    return *this;
  }

  template <class U>
  auto is() const -> bool {
    const auto tag = Imp::template tag<U>();
    return _tag == tag;
  }

  template <class U>
  auto as() const -> const U& {
    assert_fmt(this->is<U>(), "Variant::as<{}>: type not match.", reflect::type_name<U>());
    return _imp.template get_unchecked<tag_t<U>::VALUE>();
  }

  template <class U>
  auto as_mut() -> U& {
    assert_fmt(this->is<U>(), "Variant::as_mut<{}>: type not match.", reflect::type_name<U>());
    return _imp.template get_unchecked_mut<tag_t<U>::VALUE>();
  }

  void map(auto&& f) const {
    _imp.map(_tag, f);
  }

  void map_mut(auto&& f) {
    _imp.map_mut(_tag, f);
  }
};

}  // namespace sfc::variant

namespace sfc {
using variant::Variant;
}  // namespace sfc
