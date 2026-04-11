#pragma once

#include "sfc/core/mod.h"

namespace sfc::tuple {

template <class... T>
struct Tuple;

void map(auto& self, auto&& f) {
  if constexpr (requires { self._0; }) f(self._0);
  if constexpr (requires { self._1; }) f(self._1);
  if constexpr (requires { self._2; }) f(self._2);
  if constexpr (requires { self._3; }) f(self._3);
  if constexpr (requires { self._4; }) f(self._4);
  if constexpr (requires { self._5; }) f(self._5);
  if constexpr (requires { self._6; }) f(self._6);
  if constexpr (requires { self._7; }) f(self._7);
  if constexpr (requires { self._8; }) f(self._8);
  if constexpr (requires { self._9; }) f(self._9);
}

template <>
struct Tuple<> {};

template <class A>
struct Tuple<A> {
  A _0;

 public:
  void map(auto&& f) const {
    tuple::map(*this, f);
  }
};

template <class A, class B>
struct Tuple<A, B> {
  A _0;
  B _1;

 public:
  void map(auto&& f) const {
    tuple::map(*this, f);
  }
};

template <class A, class B, class C>
struct Tuple<A, B, C> {
  A _0;
  B _1;
  C _2;

 public:
  void map(auto&& f) const {
    tuple::map(*this, f);
  }
};

template <class... T>
Tuple(const T&...) -> Tuple<T...>;

}  // namespace sfc::tuple

namespace sfc {
using tuple::Tuple;
}  // namespace sfc
