#pragma once

#include "sfc/core/mod.h"

namespace sfc::tuple {

template <usize... I>
struct idx_t {};

template <usize I>
struct idx_t<I> {
  static constexpr const auto VALUE = I;
};

#if __has_builtin(__make_integer_seq)
template <class T, T... I>
struct _idxs_helper {
  using Type = idx_t<I...>;
};

template <usize N>
using idx_seq_t = typename __make_integer_seq<_idxs_helper, usize, N>::Type;
#else
template <usize N>
using idx_seq_t = idx_t<__integer_pack(N)...>;
#endif

#if __has_builtin(__type_pack_element)
template <usize I, typename... T>
using element_t = __type_pack_element<I, T...>;
#else
namespace sfc {
template <usize I, typename... T>
struct _NthType;

template <class T0, class... Ts>
struct _NthType<0, T0, Ts...> {
  using Type = T0;
};

template <class T0, class T1, class... U>
struct _NthType<1, T0, T1, U...> {
  using Type = T1;
};

template <class T0, class T1, class T2, class... U>
struct _NthType<2, T0, T1, T2, U...> {
  using Type = T2;
};

template <usize I, class T0, class T1, class T2, class... U>
struct _NthType<I, T0, T1, T2, U...> : _NthType<I - 3, U...> {};

}  // namespace sfc

template <usize I, typename... T>
using element_t = typename sfc::_NthType<I, T...>::Type;
#endif

namespace detail {

template <class I, class... T>
struct Tuple;

template <usize I, class T>
struct Entry {
  T _0;
};

template <usize... I, class... T>
struct Tuple<idx_t<I...>, T...> : Entry<I, T>... {
 public:
  Tuple(T... vals) : Entry<I, T>{static_cast<T&&>(vals)}... {}

  template <usize J>
  auto get() const -> auto& {
    using U = __type_pack_element<J, T...>;
    return static_cast<const Entry<J, U>&>(*this)._0;
  }

  template <usize J>
  auto get() -> auto& {
    using U = __type_pack_element<J, T...>;
    return static_cast<Entry<J, U>&>(*this)._0;
  }

  void map(auto&& f) const {
    (f(this->get<I>()), ...);
  }

  void map_mut(auto&& f) {
    (f(this->get<I>()), ...);
  }

  void imap(auto&& f) const {
    (f(I, this->get<I>()), ...);
  }

  void imap_mut(auto&& f) {
    (f(I, this->get<I>()), ...);
  }
};

}  // namespace detail

template <class... T>
class Tuple : detail::Tuple<idx_seq_t<sizeof...(T)>, T...> {
  using Inn = detail::Tuple<idx_seq_t<sizeof...(T)>, T...>;

 public:
  using Inn::Inn;

  using Inn::get;

  using Inn::map;
  using Inn::map_mut;

  using Inn::imap;
  using Inn::imap_mut;
};

template <class... T>
Tuple(T...) -> Tuple<T...>;

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
