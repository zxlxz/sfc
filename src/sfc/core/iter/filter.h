#pragma once

#include "iterator.h"

namespace sfc::iter {

template <class I, class P>
struct Filter : Iterator<Filter<I, P>, typename I::Item> {
  using Item = typename I::Item;
  I _iter;
  P _pred;

 public:
  explicit Filter(I iter, P pred) : _iter{static_cast<I&&>(iter)}, _pred{static_cast<P&&>(pred)} {}

  [[sfc_inline]] auto next() -> Option<Item> {
    return _iter.find(_pred);
  }

  [[sfc_inline]] auto next_back() -> Option<Item> {
    return _iter.rfind(_pred);
  }
};

template <class I, class T>
template <class P>
auto Iterator<I, T>::filter(P pred) {
  return Filter<I, P>{static_cast<I&&>(*this), static_cast<P&&>(pred)};
}

}  // namespace sfc::iter
