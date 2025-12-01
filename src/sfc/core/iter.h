#pragma once

#include "sfc/core/mem.h"
#include "sfc/core/trait.h"
#include "sfc/core/tuple.h"
#include "sfc/core/option.h"

namespace sfc::iter {

template <class I>
struct Rev;

template <class I, class P>
struct Filter;

template <class I, class F>
struct Map;

template <class T>
struct Iterator {
  using Item = T;

  auto find(this auto&& self, auto&& pred) noexcept -> Option<Item> {
    while (auto x = self.next()) {
      if (pred(*x)) {
        return x;
      }
    }
    return {};
  }

  auto rfind(this auto&& self, auto&& pred) noexcept -> Option<Item> {
    while (auto x = self.next_back()) {
      if (pred(*x)) {
        return x;
      }
    }
    return {};
  }

  auto position(this auto&& self, auto&& pred) noexcept -> Option<usize> {
    auto idx = 0UL;
    while (auto x = self.next()) {
      if (pred(*x)) {
        return idx;
      }
      ++idx;
    }

    return {};
  }

  auto rposition(this auto&& self, auto&& pred) noexcept -> Option<usize> {
    auto idx = self.len() - 1;
    while (auto x = self.next_back()) {
      if (pred(*x)) {
        return idx;
      }
      --idx;
    }
    return {};
  }

  void for_each(this auto&& self, auto&& f) {
    while (auto x = self.next()) {
      f(*x);
    }
  }

  void for_each_idx(this auto&& self, auto&& f) {
    auto i = 0UL;
    while (auto x = self.next()) {
      f(i, *x);
      ++i;
    }
  }

  template <class B>
  auto fold(this auto&& self, B init, auto&& f) -> B {
    auto accum = static_cast<B&&>(init);
    while (auto x = self.next()) {
      accum = f(accum, *x);
    }
    return accum;
  }

  template <class F, class B = trait::invoke_t<F(Item, Item)>>
  auto reduce(this auto&& self, F&& f) -> Option<B> {
    if (!self) {
      return {};
    }

    if constexpr (!trait::ref_<B>) {
      auto res_val = self.next().unwrap();
      while (auto x = self.next()) {
        res_val = f(res_val, *x);
      }
      return res_val;
    } else {
      auto res_ptr = &self.next().unwrap();
      while (auto x = self.next()) {
        res_ptr = &f(*res_ptr, *x);
      }
      return *res_ptr;
    }
  }

  auto all(this auto&& self, auto&& f) -> bool {
    return !self.position([&](auto& x) { return !f(x); });
  }

  auto any(this auto&& self, auto&& f) -> bool {
    return self.position(f).is_some();
  }

  auto count(this auto&& self) -> usize {
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

  auto min(this auto&& self) -> Option<Item> {
    return self.reduce([](auto& a, auto& b) -> Item { return a < b ? a : b; });
  }

  auto max(this auto&& self) -> Option<Item> {
    return self.reduce([](auto& a, auto& b) -> Item { return a > b ? a : b; });
  }

  auto min_by_key(this auto&& self, auto&& f) -> Option<Item> {
    return self.reduce([&](auto& a, auto& b) -> Item { return f(a) < f(b) ? a : b; });
  }

  auto max_by_key(this auto&& self, auto&& f) -> Option<Item> {
    return self.reduce([&](auto& a, auto& b) -> Item { return f(a) > f(b) ? a : b; });
  }

  template <class S = trait::decay_t<Item>>
  auto sum(this auto&& self, S init = 0) -> S {
    return self.fold(init, [](auto a, auto b) { return a + b; });
  }

  template <class S = trait::decay_t<Item>>
  auto product(this auto&& self, S init = 1) -> S {
    return self.fold(init, [](auto a, auto b) { return a * b; });
  }

  template <class B>
  auto collect(this auto&& self) -> B {
    return B::from_iter(static_cast<decltype(self)&&>(self));
  }
};

}  // namespace sfc::iter
