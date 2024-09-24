#pragma once

#include "iterator.h"

namespace sfc::iter {

template <class A, class B>
struct Zip : Iterator<Zip<A, B>, tuple::Tuple<typename A::Item, typename B::Item>> {
  using Item = tuple::Tuple<typename A::Item, typename B::Item>;

  A _a;
  B _b;

 public:
  Zip(A a, B b) : _a{static_cast<A&&>(a)}, _b{static_cast<B&&>(b)} {}

  auto next() -> option::Option<Item> {
    auto a = _a.next();
    if (!a) return {};
    auto b = _b.next();
    if (!b) return {};
    return Item{a.unwrap_unchecked(), b.unwrap_unchecked()};
  }
};

template <class A, class B>
Zip(A, B) -> Zip<A, B>;

}  // namespace sfc::iter
