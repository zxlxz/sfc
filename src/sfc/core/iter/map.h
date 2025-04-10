#pragma once

#include "iterator.h"

namespace sfc::iter {

template <class I, class F>
struct Map {
  using Item = decltype(declval<F>()(declval<I::Item>()));
  I _iter;
  F _func;

 public:
  auto len() const -> usize {
    return _iter.len();
  }

  auto next() -> Option<Item> {
    return _iter.next().map(_func);
  }

  auto next_back() -> Option<Item> {
    return _iter.next_back().map(_func);
  }

  auto operator->() -> iter::Iterator<Map>* {
    return static_cast<iter::Iterator<Map>*>(this);
  }
};

template <class I>
template <class F>
auto Iterator<I>::map(F f) -> Map<I, F> {
  return {static_cast<I&&>(*this), static_cast<F&&>(f)};
}

}  // namespace sfc::iter
