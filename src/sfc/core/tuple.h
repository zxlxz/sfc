#pragma once

#include "mod.h"

namespace sfc::tuple {

template <usize... I>
struct idx_t {};

template <usize I>
struct idx_t<I> {
  static constexpr const auto VALUE = I;
};

template <usize I, class... Ts>
struct Element;

#ifdef __clang__
template <usize I, class... T>
struct Element {
  using Type = __type_pack_element<I, T...>;
};
#else
template <usize I, class U, class... T>
struct Element<I, U, T...> : Element<I - 1, T...> {};

template <class U, class... T>
struct Element<0, U, T...> {
  using Type = U;
};
#endif

namespace detail {

template <class T, usize... I>
struct Idxs {
  using Type = idx_t<I...>;
};

// clang-format off
#if __cplusplus < 202002L
template<usize N>struct MakeSeq;
template<>struct MakeSeq<0> : Idxs<usize> {};
template<>struct MakeSeq<1> : Idxs<usize,0> {};
template<>struct MakeSeq<2> : Idxs<usize,0,1> {};
template<>struct MakeSeq<3> : Idxs<usize,0,1,2> {};
template<>struct MakeSeq<4> : Idxs<usize,0,1,2,3> {};
template<>struct MakeSeq<5> : Idxs<usize,0,1,2,3,4> {};
template<>struct MakeSeq<6> : Idxs<usize,0,1,2,3,4,5> {};
template<>struct MakeSeq<7> : Idxs<usize,0,1,2,3,4,5,6> {};
template<>struct MakeSeq<8> : Idxs<usize,0,1,2,3,4,5,6,7> {};
template<>struct MakeSeq<9> : Idxs<usize,0,1,2,3,4,5,6,7,8> {};
template<>struct MakeSeq<10>: Idxs<usize,0,1,2,3,4,5,6,7,8,9> {};
template<>struct MakeSeq<11>: Idxs<usize,0,1,2,3,4,5,6,7,8,9,10> {};
template<>struct MakeSeq<12>: Idxs<usize,0,1,2,3,4,5,6,7,8,9,10,11> {};
template<>struct MakeSeq<13>: Idxs<usize,0,1,2,3,4,5,6,7,8,9,10,11,12> {};
template<>struct MakeSeq<14>: Idxs<usize,0,1,2,3,4,5,6,7,8,9,10,11,12,13> {};
template<>struct MakeSeq<15>: Idxs<usize,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14> {};
template<>struct MakeSeq<16>: Idxs<usize,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15> {};
template<>struct MakeSeq<17>: Idxs<usize,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16> {};
template<>struct MakeSeq<18>: Idxs<usize,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17> {};
template<>struct MakeSeq<19>: Idxs<usize,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18> {};
template<>struct MakeSeq<20>: Idxs<usize,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19> {};
#endif
// clang-format on

template <usize I, class T>
struct Entry {
  T _0;
};

template <class I, class... T>
struct Tuple;

template <usize... I, class... T>
struct Tuple<idx_t<I...>, T...> : Entry<I, T>... {
  static constexpr usize COUNT = sizeof...(T);

 public:
  Tuple(T... vals) : Entry<I, T>{static_cast<T&&>(vals)}... {}

  template <usize K>
  [[sfc_inline]] auto get() const -> auto& {
    using U = typename Element<K, T...>::Type;
    return static_cast<const Entry<K, U>&>(*this)._0;
  }

  template <usize K>
  [[sfc_inline]] auto get() -> auto& {
    using U = typename Element<K, T...>::Type;
    return static_cast<Entry<K, U>&>(*this)._0;
  }

  void map(auto&& f) const {
    (f(this->get<I>()), ...);
  }

  void map_mut(auto&& f) {
    (f(this->get<I>()), ...);
  }

  void imap(auto&& f) const {
    (f(idx_t<I>{}, this->get<I>()), ...);
  }

  void imap_mut(auto&& f) {
    (f(idx_t<I>{}, this->get<I>()), ...);
  }
};

}  // namespace detail

template <usize I, class... T>
using element_t = typename Element<I, T...>::Type;

#ifdef __clang__
template <usize N>
using idx_seq_t = __make_integer_seq<detail::Idxs, usize, N>::Type;
#elif defined(__GNUC__)
#if __GNUC__ >= 9
template <usize N>
using idx_seq_t = idx_t<__integer_pack(N)...>;
#else
template <usize N>
using idx_seq_t = typename detail::MakeSeq<N>::Type;
#endif
#endif

template <class... T>
class Tuple : detail::Tuple<idx_seq_t<sizeof...(T)>, T...> {
  using Inn = detail::Tuple<idx_seq_t<sizeof...(T)>, T...>;

 public:
  using Inn::COUNT;
  using Inn::Inn;

  using Inn::get;
  using Inn::map;
  using Inn::map_mut;

  using Inn::imap;
  using Inn::imap_mut;
};

#if __cplusplus >= 202002L
template <class... T>
Tuple(T...) -> Tuple<T...>;
#endif

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

namespace sfc {
using tuple::idx_seq_t;
using tuple::idx_t;
using tuple::Tuple;
}  // namespace sfc
