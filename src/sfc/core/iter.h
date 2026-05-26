#pragma once

#include "sfc/core/mem.h"
#include "sfc/core/option.h"

namespace sfc::iter {

template <class I>
struct Rev;

template <class I, class F>
struct Map;

template <class I, class P>
struct Filter;

struct Iterator {
  template <class Self>
  auto find(this Self self, auto&& pred) noexcept -> Option<typename Self::Item> {
    while (auto x = self.next()) {
      if (pred(*x)) {
        return x;
      }
    }
    return {};
  }

  template <class Self>
  auto rfind(this Self self, auto&& pred) noexcept -> Option<typename Self::Item> {
    while (auto x = self.next_back()) {
      if (pred(*x)) {
        return x;
      }
    }
    return {};
  }

  template <class Self>
  auto position(this Self self, auto&& pred) noexcept -> Option<usize> {
    auto idx = 0UL;
    while (auto x = self.next()) {
      if (pred(*x)) {
        return idx;
      }
      ++idx;
    }

    return {};
  }

  template <class Self>
  auto rposition(this Self self, auto&& pred) noexcept -> Option<usize> {
    auto idx = self.len() - 1;
    while (auto x = self.next_back()) {
      if (pred(*x)) {
        return idx;
      }
      --idx;
    }
    return {};
  }

  template <class Self>
  void for_each(this Self self, auto&& f) {
    while (auto x = self.next()) {
      f(*x);
    }
  }

  template <class Self>
  void for_each_idx(this Self self, auto&& f) {
    auto i = 0UL;
    while (auto x = self.next()) {
      f(i, *x);
      ++i;
    }
  }

  template <class Self, class B>
  auto fold(this Self self, B init, auto&& f) -> B {
    auto accum = init;
    while (auto x = self.next()) {
      accum = f(accum, *x);
    }
    return accum;
  }

  template <class Self, class F>
  auto reduce(this Self self, F&& f) -> Option<typename Self::Item> {
    using Item = typename Self::Item;

    auto opt = self.next();
    if (!opt) {
      return {};
    }

    if constexpr (!trait::same_<Item, Item&>) {
      auto res_val = mem::move(opt).unwrap();
      while (auto x = self.next()) {
        res_val = f(res_val, *x);
      }
      return res_val;
    } else {
      auto res_ptr = &*opt;
      while (auto x = self.next()) {
        res_ptr = &f(*res_ptr, *x);
      }
      return *res_ptr;
    }
  }

  template <class Self>
  auto all(this Self self, auto&& f) -> bool {
    return !self.position([&](auto& x) { return !f(x); });
  }

  template <class Self>
  auto any(this Self self, auto&& f) -> bool {
    return self.position(f).is_some();
  }

  template <class Self>
  auto count(this Self self) -> usize {
    if constexpr (requires { self.len(); }) {
      return self.len();
    }
    auto cnt = 0UL;
    while (auto x = self.next()) {
      ++cnt;
    }
    return cnt;
  }

  template <class Self>
  auto min(this Self self) -> Option<typename Self::Item> {
    using Item = typename Self::Item;
    return self.reduce([](auto& a, auto& b) -> Item { return a < b ? a : b; });
  }

  template <class Self>
  auto max(this Self self) -> Option<typename Self::Item> {
    using Item = typename Self::Item;
    return self.reduce([](auto& a, auto& b) -> Item { return a > b ? a : b; });
  }

  template <class Self>
  auto min_by_key(this Self self, auto&& f) -> Option<typename Self::Item> {
    using Item = typename Self::Item;
    return self.reduce([&](auto& a, auto& b) -> Item { return f(a) < f(b) ? a : b; });
  }

  template <class Self>
  auto max_by_key(this Self self, auto&& f) -> Option<typename Self::Item> {
    using Item = typename Self::Item;
    return self.reduce([&](auto& a, auto& b) -> Item { return f(a) > f(b) ? a : b; });
  }

  template <class B, class Self>
  auto collect(this Self self) -> B {
    return B::from_iter(mem::move(self));
  }

  template <class Self, class F>
  auto map(this Self self, F f) -> Map<Self, F> {
    return Map<Self, F>{mem::move(self), mem::move(f)};
  }

  template <class Self, class P>
  auto filter(this Self self, P pred) -> Filter<Self, P> {
    return Filter<Self, P>{mem::move(self), mem::move<P>(pred)};
  }
};

template <class I, class F>
struct Map : Iterator {
  using Item = typename ops::invoke_t<F(typename I::Item)>;
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
struct Filter : Iterator {
  using Item = typename I::Item;

  I _iter;
  P _pred;

 public:
  auto next() noexcept -> Option<Item> {
    while (auto x = _iter.next()) {
      if (_pred(*x)) {
        return x;
      }
    }
    return {};
  }
};

}  // namespace sfc::iter
