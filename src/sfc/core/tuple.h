#pragma once

#include "sfc/core/mod.h"

namespace sfc::tuple {

template <usize I, class... T>
using element_t = __type_pack_element<I, T...>;

template <int I, class T>
struct Entry {
  T _0;
};

template <class, int... I>
struct Idxs {};

template <class I, class... T>
struct TupleImpl;

template <int... I, class... T>
struct TupleImpl<Idxs<int, I...>, T...> : Entry<I, T>... {
  TupleImpl(T&&... args) : Entry<I, T>{static_cast<T&&>(args)}... {}

  TupleImpl(TupleImpl&&) noexcept = default;
  TupleImpl(const TupleImpl&) = default;

  TupleImpl& operator=(TupleImpl&&) noexcept = default;
  TupleImpl& operator=(const TupleImpl&) = default;

  void map(auto&& f) const {
    (void)(f(static_cast<const Entry<I, T>&>(*this)._0), ...);
  }

  void map_mut(auto&& f) {
    (void)(f(static_cast<Entry<I, T>&>(*this)._0), ...);
  }
};

template <class... T>
struct Tuple {
  TupleImpl<__make_integer_seq<Idxs, int, sizeof...(T)>, T...> _impl;

 public:
  Tuple(T... args) : _impl{static_cast<T&&>(args)...} {}

  template <int I>
  auto get() -> element_t<I, T...>& {
    using U = element_t<I, T...>;
    return static_cast<Entry<I, U>&>(_impl)._0;
  }

  template <int I>
  auto get() const -> const element_t<I, T...>& {
    using U = element_t<I, T...>;
    return static_cast<const Entry<I, U>&>(_impl)._0;
  }

  void map(auto&& f) const {
    return _impl.map(f);
  }

  void map_mut(auto&& f) {
    _impl.map_mut(f);
  }

  void fmt(auto& f) const {
    auto x = f.debug_tuple();
    _impl.map([&](const auto& e) { x.entry(e); });
  }
};

}  // namespace sfc::tuple

namespace std {

#ifdef _LIBCPP_ABI_NAMESPACE
inline namespace _LIBCPP_ABI_NAMESPACE {
#endif

template <class>
struct tuple_size;

template <class... T>
struct tuple_size<sfc::tuple::Tuple<T...>> {
  static constexpr auto value = sizeof...(T);
};

template <class... T>
struct tuple_size<const sfc::tuple::Tuple<T...>> {
  static constexpr auto value = sizeof...(T);
};

template <sfc::usize, class>
struct tuple_element;

template <sfc::usize I, class... T>
struct tuple_element<I, sfc::tuple::Tuple<T...>> {
  using type = sfc::tuple::element_t<I, T...>;
};

template <sfc::usize I, class... T>
struct tuple_element<I, const sfc::tuple::Tuple<T...>> {
  using type = const sfc::tuple::element_t<I, T...>;
};

#ifdef _LIBCPP_ABI_NAMESPACE
}  // namespace _LIBCPP_ABI_NAMESPACE
#endif
}  // namespace std
