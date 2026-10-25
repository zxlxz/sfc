#pragma once

#include "sfc/core/mem.h"
#include "sfc/core/option.h"

namespace sfc::iter {

template <class I>
struct Rev;

template <class I>
struct Take;

template <class I, class F>
struct Map;

template <class I, class P>
struct Filter;

template <class T>
struct Iterator {
  using Item = T;

 public:
  // Iterator::next
  // auto next(this auto& self)  -> Option<Item>;

  // DoubleEndedIterator::next_back
  // auto next_back(this auto& self)  -> Option<Item>;

  // ExactSizeIterator::len
  // auto len(this const auto& self)  -> usize;

  // ExactSizeIterator::is_empty
  auto is_empty(this const auto& self) -> bool {
    return self.len() == 0;
  }

 public:
  auto advance_by(this auto&& self, usize n) noexcept -> usize {
    for (auto i = usize{0}; i < n; ++i) {
      if (!self.next()) {
        return i;
      }
    }
    return n;
  }

  auto find(this auto&& self, auto&& pred) noexcept -> Option<Item> {
    while (auto x = self.next()) {
      if (pred(*x)) return x;
    }
    return {};
  }

  auto rfind(this auto&& self, auto&& pred) noexcept -> Option<Item> {
    while (auto x = self.next_back()) {
      if (pred(*x)) return x;
    }
    return {};
  }

  auto position(this auto&& self, auto&& pred) noexcept -> Option<usize> {
    auto idx = 0UL;
    while (auto x = self.next()) {
      if (pred(*x)) return idx;
      ++idx;
    }

    return {};
  }

  auto rposition(this auto&& self, auto&& pred) noexcept -> Option<usize> {
    auto n = self.len();
    if (n == 0) {
      return {};
    }
    while (auto x = self.next_back()) {
      if (pred(*x)) return n - 1;
      --n;
    }
    return {};
  }

  auto all(this auto&& self, auto&& f) -> bool {
    return self.find([&](auto& x) { return !f(x); }).is_none();
  }

  auto any(this auto&& self, auto&& f) -> bool {
    return self.find(f).is_some();
  }

  void try_for_each(this auto&& self, auto&& f) {
    while (auto x = self.next()) {
      if (!f(*x)) break;
    }
  }

 public:
  void for_each(this auto self, auto&& f) {
    while (auto x = self.next()) {
      f(*x);
    }
  }

  template <class B>
  auto fold(this auto self, B init, auto&& f) -> B {
    auto accum = init;
    while (auto x = self.next()) {
      accum = f(accum, *x);
    }
    return accum;
  }

  template <class F>
  auto reduce(this auto self, F&& f) -> Option<Item> {
    auto res = self.next();
    if (!res) {
      return {};
    }

    while (auto x = self.next()) {
      res = f(*res, *x);
    }
    return res;
  }

  auto count(this auto self) -> usize {
    if constexpr (requires { self.len(); }) {
      return self.len();
    } else {
      auto cnt = 0UL;
      while (auto x = self.next()) {
        ++cnt;
      }
      return cnt;
    }
  }

  auto min(this auto self) -> Option<Item> {
    return self.reduce([](auto& a, auto& b) -> Item { return a < b ? a : b; });
  }

  auto max(this auto self) -> Option<Item> {
    return self.reduce([](auto& a, auto& b) -> Item { return a > b ? a : b; });
  }

  auto min_by_key(this auto self, auto&& f) -> Option<Item> {
    return self.reduce([&](auto& a, auto& b) -> Item { return f(a) < f(b) ? a : b; });
  }

  auto max_by_key(this auto self, auto&& f) -> Option<Item> {
    return self.reduce([&](auto& a, auto& b) -> Item { return f(a) > f(b) ? a : b; });
  }

  template <class B>
  auto collect(this auto self, B res = B{}) -> B {
    res.extend(mem::move(self));
    return res;
  }

  auto& collect_into(this auto self, auto& res) {
    res.extend(mem::move(self));
    return res;
  }

 public:
  template <class Self>
  auto take(this Self self, usize n) -> Take<Self> {
    return Take<Self>{{}, mem::move(self), n};
  }

  template <class Self, class F>
  auto map(this Self self, F f) -> Map<Self, F> {
    return Map<Self, F>{{}, mem::move(self), mem::move(f)};
  }

  template <class Self, class P>
  auto filter(this Self self, P pred) -> Filter<Self, P> {
    return Filter<Self, P>{{}, mem::move(self), mem::move<P>(pred)};
  }
};

// make DoubleEndedIterator an alias for Iterator
// let c++ compiler handle DoubleEndedIterator functions
template <class T>
using DoubleEndedIterator = Iterator<T>;

template <class I>
struct Take : Iterator<typename I::Item> {
  using Item = typename I::Item;
  I _iter;
  usize _n;

 public:
  auto next() noexcept -> Option<Item> {
    if (_n == 0) {
      return {};
    }
    _n -= 1;
    return _iter.next();
  }
};

template <class I, class F>
struct Map : Iterator<FnOut<F, typename I::Item>> {
  using Item = FnOut<F, typename I::Item>;
  I _iter;
  F _func;

 public:
  auto next() noexcept -> Option<Item> {
    if (auto x = _iter.next()) {
      return Item{_func(*x)};
    }
    return {};
  }
};

template <class I, class P>
struct Filter : Iterator<typename I::Item> {
  using Item = typename I::Item;
  I _iter;
  P _pred;

 public:
  auto next() noexcept -> Option<Item> {
    while (auto x = _iter.next()) {
      if (_pred(*x)) return x;
    }
    return {};
  }

  auto next_back() noexcept -> Option<Item> {
    while (auto x = _iter.next_back()) {
      if (_pred(*x)) return x;
    }
    return {};
  }
};

}  // namespace sfc::iter
