#pragma once

#include "iterator.h"

namespace sfc::iter {

template <class F, class... Args>
using invoke_result_t = decltype(declval<F>()(declval<Args>()...));

template <class I, class F>
struct Map : Iterator<Map<I, F>, invoke_result_t<F, typename I::Item>> {
  using Item = invoke_result_t<F, typename I::Item>;
  I _iter;
  F _func;

 public:
  explicit Map(I iter, F func) : _iter{static_cast<I&&>(iter)}, _func{static_cast<F&&>(func)} {}

  [[sfc_inline]] auto len() const -> usize {
    return _iter.len();
  }

  [[sfc_inline]] auto next() -> Option<Item> {
    auto val = _iter.next();
    if (!val) return {};
    return _func(mem::move(val).unwrap());
  }

  [[sfc_inline]] auto next_back() -> Option<Item> {
    auto val = _iter.next_back();
    if (!val) return {};
    return _func(mem::move(val).unwrap());
  }
};

template <class I, class T>
template <class F>
auto Iterator<I, T>::map(F func) {
  return Map<I, F>{static_cast<I&&>(*this), static_cast<F&&>(func)};
}

}  // namespace sfc::iter
