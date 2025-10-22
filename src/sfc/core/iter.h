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

  auto find(this auto&& self, auto&& pred) -> Option<Item> {
    while (auto x = self.next()) {
      if (pred(*x)) {
        return x;
      }
    }
    return {};
  }

  auto rfind(this auto&& self, auto&& pred) -> Option<Item> {
    while (auto x = self.next_back()) {
      if (pred(*x)) {
        return x;
      }
    }
    return {};
  }

  auto position(this auto&& self, auto&& pred) -> Option<usize> {
    for (usize idx = 0UL; auto x = self.next(); ++idx) {
      if (pred(*x)) {
        return idx;
      }
    }

    return {};
  }

  auto rposition(this auto&& self, auto&& pred) -> Option<usize> {
    for (auto idx = self.len() - 1; auto x = self.next_back(); --idx) {
      if (pred(*x)) {
        return idx;
      }
    }
    return {};
  }

  void for_each(this auto&& self, auto&& f) {
    while (auto x = self.next()) {
      f(*x);
    }
  }

  void for_each_idx(this auto&& self, auto&& f) {
    for (auto i = 0UL; auto x = self.next(); ++i) {
      f(i, *x);
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
  auto reduce(this auto self, F&& f) -> Option<B> {
    auto first = self.next();
    if (!first) {
      return {};
    }

    if constexpr (!__is_same(B, B&)) {
      auto res_val = mem::move(first).unwrap();
      while (auto x = self.next()) {
        res_val = f(res_val, *x);
      }
      return res_val;
    } else {
      auto res_ptr = &mem::move(first).unwrap();
      while (auto x = self.next()) {
        res_ptr = &f(*res_ptr, *x);
      }
      return *res_ptr;
    }
  }

  auto all(this auto self, auto&& f) -> bool {
    return !self.position([&](auto& x) { return !f(x); });
  }

  auto any(this auto self, auto&& f) -> bool {
    return self.position(f).is_some();
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
    return self.reduce([](auto&& a, auto&& b) -> auto& { return a < b ? a : b; });
  }

  auto max(this auto self) -> Option<Item> {
    return self.reduce([](auto&& a, auto&& b) -> auto& { return a > b ? a : b; });
  }

  auto min_by_key(this auto self, auto&& f) -> Option<Item> {
    return self.reduce([&](auto&& a, auto&& b) -> auto& { return f(a) < f(b) ? a : b; });
  }

  auto max_by_key(this auto self, auto&& f) -> Option<Item> {
    return self.reduce([&](auto&& a, auto&& b) -> auto& { return f(a) > f(b) ? a : b; });
  }

  template <class S = trait::decay_t<Item>>
  auto sum(this auto&& self, S init = 0) -> S {
    return self.fold(init, [](const auto& a, const auto& b) { return a + b; });
  }

  template <class S = trait::decay_t<Item>>
  auto product(this auto&& self, S init = 1) -> S {
    return self.fold(init, [](const auto& a, const auto& b) { return a * b; });
  }

  template <class B>
  auto collect(this auto&& self) -> B {
    return B::from_iter(static_cast<decltype(self)&&>(self));
  }

  template <class Self>
  auto rev(this Self self) -> Rev<Self> {
    return Rev{{}, static_cast<Self&&>(self)};
  }

  template <class Self, class F>
  auto map(this Self self, F f) -> Map<Self, F> {
    return {{}, static_cast<Self&&>(self), static_cast<F&&>(f)};
  }
};

template <class I>
struct Rev : Iterator<typename I::Item> {
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
};

template <class I, class F>
struct Map : Iterator<trait::invoke_t<F(typename I::Item)>> {
  using Item = trait::invoke_t<F(typename I::Item)>;
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
};

template <class A, class B>
struct Zip : Iterator<typename A::Item> {
  using Item = Tuple<typename A::Item, typename B::Item>;

  A _a;
  B _b;

 public:
  auto next() -> Option<Item> {
    auto a = _a.next();
    if (!a) {
      return {};
    }
    auto b = _b.next();
    if (!b) {
      return {};
    }
    return Item{mem::move(a).unwrap(), mem::move(b).unwrap()};
  }

  auto next_back() -> Option<Item> {
    auto a = _a.next_back();
    if (!a) {
      return {};
    }
    auto b = _b.next_back();
    if (!b) {
      return {};
    }
    return Item{mem::move(a).unwrap(), mem::move(b).unwrap()};
  }

  auto operator->() const -> const iter::Iterator<Zip>* {
    return this;
  }

  auto operator->() -> iter::Iterator<Zip>* {
    return this;
  }
};

template <class A, class B>
auto zip(A a, B b) -> Zip<A, B> {
  return {{}, static_cast<A&&>(a), static_cast<B&&>(b)};
}

}  // namespace sfc::iter
