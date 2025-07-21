#pragma once

#include "sfc/core/mem.h"
#include "sfc/core/option.h"
#include "sfc/core/tuple.h"

namespace sfc::iter {

template <class I>
struct Rev;

template <class I, class P>
struct Filter;

template <class I, class F>
struct Map;

template <class I, class T>
struct Iterator {
  using Item = T;

 public:
  auto nth(usize n) -> Option<Item> {
    auto& self = static_cast<I&>(*this);

    for (auto i = 0UL; i < n; ++i) {
      self.next();
    }
    return self.next();
  }

  void for_each(auto&& f) {
    auto& self = static_cast<I&>(*this);

    for (; auto x = self.next();) {
      f(x.get_unchecked_mut());
    }
  }

  void for_each_idx(auto&& f) {
    auto& self = static_cast<I&>(*this);

    for (auto i = 0UL; auto x = self.next(); ++i) {
      f(i, x.get_unchecked_mut());
    }
  }

  template <class B>
  auto fold(B init, auto&& f) -> B {
    auto& self = static_cast<I&>(*this);

    if constexpr (!__is_same(B, B&)) {
      auto accum = static_cast<B&&>(init);
      for (; auto x = self.next();) {
        accum = f(accum, x.get_unchecked_mut());
      }
      return accum;
    } else {
      auto accum_ptr = &init;
      for (; auto x = self.next();) {
        accum_ptr = &f(*accum_ptr, x.get_unchecked_mut());
      }
      return *accum_ptr;
    }
  }

  auto reduce(auto&& f) -> Option<Item> {
    auto& self = static_cast<I&>(*this);

    auto first = self.next();
    if (!first) {
      return {};
    }

    return this->fold<Item>(mem::move(first).unwrap(), f);
  }

  auto find(auto&& pred) -> Option<Item> {
    auto& self = static_cast<I&>(*this);

    for (; auto x = self.next();) {
      if (pred(x.get_unchecked_mut())) {
        return x;
      }
    }
    return {};
  }

  auto rfind(auto&& pred) -> Option<Item> {
    auto& self = static_cast<I&>(*this);

    for (; auto x = self.next_back();) {
      if (pred(x.get_unchecked_mut())) {
        return x;
      }
    }
    return {};
  }

  auto position(auto&& pred) -> Option<usize> {
    auto& self = static_cast<I&>(*this);

    for (usize idx = 0UL; auto x = self.next(); ++idx) {
      if (pred(x.get_unchecked_mut())) {
        return Option{idx};
      }
    }

    return {};
  }

  auto rposition(auto&& pred) -> Option<usize> {
    auto& self = static_cast<I&>(*this);

    for (auto idx = self.len() - 1; auto x = self.next_back(); --idx) {
      if (pred(x.get_unchecked_mut())) {
        return idx;
      }
    }
    return {};
  }

  auto all(auto&& f) -> bool {
    return !this->position([&](auto& x) { return !f(x); });
  }

  auto any(auto&& f) -> bool {
    return this->position(f);
  }

  auto count(auto&& f) -> usize {
    auto accum = usize{0};
    this->for_each([&](auto& x) mutable { f(x) ? accum += 1 : void(); });
    return accum;
  }

  auto min() -> Item {
    return this->reduce([](auto&& a, auto&& b) -> auto& { return a < b ? a : b; });
  }

  auto max() -> Item {
    return this->reduce([](auto&& a, auto&& b) -> auto& { return a > b ? a : b; });
  }

  auto min_by_key(auto&& f) {
    return this->reduce([&](auto&& a, auto&& b) -> auto& { return f(a) < f(b) ? a : b; });
  }

  auto max_by_key(auto&& f) {
    return this->reduce([&](auto&& a, auto&& b) -> auto& { return f(a) > f(b) ? a : b; });
  }

  auto sum() {
    using U = decltype(declval<Item>() + declval<Item>());
    auto init = U{0};

    return this->fold(init, [](const auto& a, const auto& b) { return a + b; });
  }

  auto product() {
    using U = decltype(declval<Item>() * declval<Item>());
    auto init = U{1};

    return this->fold(init, [](const auto& a, const auto& b) { return a * b; });
  }

  template <class B>
  auto collect() && -> B {
    return B::from_iter(static_cast<I&&>(*this));
  }

  auto rev() && -> Rev<I> {
    return Rev{{}, static_cast<I&&>(*this)};
  }

  template <class F>
  auto map(F f) && -> Map<I, F> {
    return Map{{}, static_cast<I&&>(*this), static_cast<F&&>(f)};
  }

  template <class P>
  auto filter(P pred) -> Filter<I, P>;
};

template <class I>
struct Rev : Iterator<Rev<I>, typename I::Item> {
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
struct Map : Iterator<Map<I, F>, decltype(declval<F>()(declval<typename I::Item>()))> {
  using Item = decltype(declval<F>()(declval<typename I::Item>()));

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
struct Zip : Iterator<Zip<A, B>, tuple::Tuple<typename A::Item, typename B::Item>> {
  using Item = tuple::Tuple<typename A::Item, typename B::Item>;

  A _a;
  B _b;

 public:
  auto next() -> option::Option<Item> {
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

  auto next_back() -> option::Option<Item> {
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
};

}  // namespace sfc::iter
