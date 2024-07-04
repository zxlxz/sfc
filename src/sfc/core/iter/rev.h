#pragma once

#include "iterator.h"

namespace sfc::iter {

template <class I>
struct Rev : Iterator<Rev<I>, typename I::Item> {
  using Item = typename I::Item;
  I _iter;

 public:
  explicit Rev(I iter) : _iter{static_cast<I&&>(iter)} {}

  [[sfc_inline]] auto len() const -> usize {
    return _iter.len();
  }

  [[sfc_inline]] auto next() -> Option<Item> {
    return _iter.next_back();
  }

  [[sfc_inline]] auto next_back() -> Option<Item> {
    return _iter.next();
  }
};

template <class I, class T>
auto Iterator<I, T>::rev() {
  return Rev<I>{static_cast<I&&>(*this)};
}

}  // namespace sfc::iter
