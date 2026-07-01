#pragma once

#include "sfc/core/mod.h"

namespace sfc::tuple {

template <class... T>
struct _TupleData;

// clang-format off
template <>
struct _TupleData<>{ };

template <class A>
struct _TupleData<A>{ A _0; };

template <class A, class B>
struct _TupleData<A, B>{ A _0; B _1;};

template <class A, class B, class C>
struct _TupleData<A, B, C> { A _0; B _1; C _2; };

template <class A, class B, class C, class D>
struct _TupleData<A, B, C, D> { A _0; B _1; C _2; D _3; };

template <class A, class B, class C, class D, class E>
struct _TupleData<A, B, C, D, E> { A _0; B _1; C _2; D _3; E _4; };

template <class A, class B, class C, class D, class E, class F>
struct _TupleData<A, B, C, D, E, F> { A _0; B _1; C _2; D _3; E _4; F _5; };

template <class A, class B, class C, class D, class E, class F, class G>
struct _TupleData<A, B, C, D, E, F, G> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; };

template <class A, class B, class C, class D, class E, class F, class G, class H>
struct _TupleData<A, B, C, D, E, F, G, H> { A _0; B _1; C _2; D _3; E _4; F _5; G _6; H _7; };
// clang-format on

template <class... T>
struct Tuple : _TupleData<T...> {
  static constexpr u32 COUNT = sizeof...(T);
  using Inn = _TupleData<T...>;

 public:
  Tuple(T... args) noexcept : Inn{(T&&)(args)...} {}
  ~Tuple() = default;

 public:
  void for_each(auto&& f) const {
    if constexpr (COUNT > 0) f(this->_0);
    if constexpr (COUNT > 1) f(this->_1);
    if constexpr (COUNT > 2) f(this->_2);
    if constexpr (COUNT > 3) f(this->_3);
    if constexpr (COUNT > 4) f(this->_4);
    if constexpr (COUNT > 5) f(this->_5);
    if constexpr (COUNT > 6) f(this->_6);
    if constexpr (COUNT > 7) f(this->_7);
  }

  void for_each_mut(auto&& f) {
    if constexpr (COUNT > 0) f(this->_0);
    if constexpr (COUNT > 1) f(this->_1);
    if constexpr (COUNT > 2) f(this->_2);
    if constexpr (COUNT > 3) f(this->_3);
    if constexpr (COUNT > 4) f(this->_4);
    if constexpr (COUNT > 5) f(this->_5);
    if constexpr (COUNT > 6) f(this->_6);
    if constexpr (COUNT > 7) f(this->_7);
  }

  void fmt(auto& f) const {
    auto imp = f.debug_tuple();
    this->for_each([&](const auto& val) { imp.field(val); });
  }
};

template <class... T>
Tuple(T...) -> Tuple<T...>;

template <class... T>
auto bind(T&... args) -> Tuple<T&...> {
  return Tuple<T&...>{args...};
}

}  // namespace sfc::tuple

namespace sfc {
using tuple::Tuple;
}  // namespace sfc
