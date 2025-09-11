#pragma once

#include "sfc/core/mod.h"

namespace sfc::tuple {

template <class T, T... I>
struct IntSeq {};

template <int I, class... T>
struct Element {};

template <class T, class... Ts>
struct Element<0, T, Ts...> {
  using Type = T;
};

template <int I, class T, class... Ts>
struct Element<I, T, Ts...> : Element<I - 1, Ts...> {};

#ifdef __clang__
template <usize I, class... T>
using element_t = __type_pack_element<I, T...>;
#else
template <usize I, class... T>
using element_t = Element<I, T...>::Type;
#endif

template <int I, class T>
struct Entry {
  T _0;
};

template <class I, class... T>
struct Inner;

template <int... I, class... T>
struct Inner<IntSeq<int, I...>, T...> : Entry<I, T>... {
  Inner(T&&... args) : Entry<I, T>{static_cast<T&&>(args)}... {}

  Inner(Inner&&) noexcept = default;
  Inner(const Inner&) = default;

  Inner& operator=(Inner&&) noexcept = default;
  Inner& operator=(const Inner&) = default;

  template <int X, class U = element_t<X, T...>>
  auto get() -> U& {
    return Entry<X, U>::_0;
  }

  template <int X, class U = element_t<X, T...>>
  auto get() const -> const U& {
    return Entry<X, U>::_0;
  }

  void map(auto&& f) const {
    (void)(f(Entry<I, T>::_0), ...);
  }

  void map_mut(auto&& f) {
    (void)(f(Entry<I, T>::_0), ...);
  }
};

template <class... T>
struct Tuple {
  using Inn = Inner<__make_integer_seq<IntSeq, int, sizeof...(T)>, T...>;
  Inn _inn;

 public:
  Tuple(T... args) : _inn{static_cast<T&&>(args)...} {}

  template <int I>
  auto get() -> element_t<I, T...>& {
    return _inn.template get<I>();
  }

  template <int I>
  auto get() const -> const element_t<I, T...>& {
    return _inn.template get<I>();
  }

  void map(auto&& f) const {
    return _inn.map(f);
  }

  void map_mut(auto&& f) {
    _inn.map_mut(f);
  }

  void fmt(auto& f) const {
    auto x = f.debug_tuple();
    _inn.map([&](const auto& e) { x.entry(e); });
  }
};

template <>
struct Tuple<> {};

}  // namespace sfc::tuple

namespace SFC_STD {

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

}  // namespace SFC_STD
