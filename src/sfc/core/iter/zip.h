#pragma once

#include "sfc/core/iter/iterator.h"

namespace sfc::iter {

template <class A, class B>
struct Zip {
  using Item = tuple::Tuple<typename A::Item, typename B::Item>;

  A _a;
  B _b;

 public:
  auto next() -> option::Option<Item> {
    auto a = _a.next();
    if (!a) {
      return {};
    }
    auto b = _b.next();
    if (!b) {
      return {};
    }
    return Item{a.unwrap(), b.unwrap()};
  }

  auto next_back() -> option::Option<Item> {
    auto a = _a.next_back();
    if (!a) {
      return {};
    }
    auto b = _b.next_back();
    if (!b) {
      return {};
    }
    return Item{a.unwrap_unchecked(), b.unwrap_unchecked()};
  }

  auto operator->() -> iter::Iterator<Zip>* {
    return static_cast<iter::Iterator<Zip>*>(this);
  }
};


}  // namespace sfc::iter
