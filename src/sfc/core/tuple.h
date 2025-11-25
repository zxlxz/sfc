#pragma once

#include "sfc/core/mod.h"

namespace sfc::tuple {

template <class T, T... I>
struct IntSeq {};

template <int I, class T>
struct Entry {
  T _0;
};

template <class I, class... T>
struct Inner;

template <int... I, class... T>
struct Inner<IntSeq<int, I...>, T...> : Entry<I, T>... {
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
  Tuple(T... args) : _inn{{static_cast<T&&>(args)}...} {}
  ~Tuple() = default;

#ifdef __clang__
  template <usize I>
  using element_t = __type_pack_element<I, T...>;
#else
  template <usize I>
  using element_t = T...[I];
#endif

  template <usize I>
  using entry_t = Entry<I, element_t<I>>;

  template <usize I>
  auto get() const -> const element_t<I>& {
    return _inn.entry_t<I>::_0;
  }

  template <usize I>
  auto get() -> element_t<I>& {
    return _inn.entry_t<I>::_0;
  }

  template <usize I>
  auto get_mut() -> element_t<I>& {
    return _inn.entry_t<I>::_0;
  }

  void map(auto&& f) const {
    return _inn.map(f);
  }

  void map_mut(auto&& f) {
    _inn.map_mut(f);
  }

 public:
  // trait: fmt::Display
  void fmt(auto& f) const {
    auto x = f.debug_tuple();
    _inn.map([&](const auto& e) { x.entry(e); });
  }

  // trait: serde::Serialize
  void serialize(auto& ser) const {
    auto imp = ser.serialize_tuple();
    this->map([&](const auto& e) { imp.serialize_element(e); });
  }
};

template <>
struct Tuple<> {};

template <class... T>
Tuple(T...) -> Tuple<T...>;

}  // namespace sfc::tuple

namespace sfc {
using tuple::Tuple;
}  // namespace sfc

namespace SFC_STD {

template <class>
struct tuple_size;

template <class... T>
struct tuple_size<sfc::Tuple<T...>> {
  static constexpr auto value = sizeof...(T);
};

template <class... T>
struct tuple_size<const sfc::Tuple<T...>> {
  static constexpr auto value = sizeof...(T);
};

template <sfc::usize, class>
struct tuple_element;

template <sfc::usize I, class... T>
struct tuple_element<I, sfc::Tuple<T...>> {
  using type = sfc::Tuple<T...>::template element_t<I>;
};

template <sfc::usize I, class... T>
struct tuple_element<I, const sfc::Tuple<T...>> {
  using type = const sfc::Tuple<T...>::template element_t<I>;
};

}  // namespace SFC_STD
