#pragma once

#include "sfc/core/iter/iterator.h"

namespace sfc::iter {

template <class I>
struct Rev {
  using Item = typename I::Item;
  I _iter;

 public:
  auto len() const -> usize {
    return _iter.len();
  }

  auto next() -> Option<Item> {
    return _iter.next_back();
  }

  auto next_back() -> Option<Item> {
    return _iter.next();
  }

  auto operator->() -> iter::Iterator<Rev> {
    return static_cast<iter::Iterator<Rev>*>(this);
  }
};

template <class Impl>
auto Iterator<Impl>::rev() -> Rev<Impl> {
  return {static_cast<Impl&&>(*this)};
}

}  // namespace sfc::iter
