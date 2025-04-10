#pragma once

#include "iterator.h"

namespace sfc::iter {

template <class I, class P>
struct Filter {
  using Item = typename I::Item;
  I _iter;
  P _pred;

 public:
  auto next() -> Option<Item> {
    return _iter.find(_pred);
  }

  auto next_back() -> Option<Item> {
    return _iter.rfind(_pred);
  }

  auto operator->() -> iter::Iterator<Filter>* {
    return static_cast<iter::Iterator<Filter>*>(this);
  }
};

}  // namespace sfc::iter
