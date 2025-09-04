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

template <class I>
struct Iterator : I {
  using Item = typename I::Item;

  Iterator() = delete;
  ~Iterator() = delete;

 public:
  void for_each(auto&& f) {
    for (; auto x = this->next();) {
      f(*x);
    }
  }

  void for_each_idx(auto&& f) {
    for (auto i = 0UL; auto x = this->next(); ++i) {
      f(i, *x);
    }
  }

  template <class B>
  auto fold(B init, auto&& f) -> B {
    auto accum = static_cast<B&&>(init);
    for (; auto x = this->next();) {
      accum = f(accum, *x);
    }
    return accum;
  }

  template <class F, class U = expr_t<F(Item, Item)>>
  auto reduce(F&& f) -> option::Option<U> {
    auto first = this->next();
    if (!first) {
      return {};
    }

    if constexpr (!__is_same(U, U&)) {
      auto res_val = static_cast<U>(*first);
      for (; auto x = this->next();) {
        res_val = f(res_val, *x);
      }
      return res_val;
    } else {
      auto res_ptr = &static_cast<U>(*first);
      for (; auto x = this->next();) {
        res_ptr = &f(*res_ptr, *x);
      }
      return *res_ptr;
    }
  }

  auto find(auto&& pred) -> option::Option<Item> {
    for (; auto x = this->next();) {
      if (pred(*x)) {
        return x;
      }
    }
    return {};
  }

  auto rfind(auto&& pred) -> option::Option<Item> {
    for (; auto x = this->next_back();) {
      if (pred(*x)) {
        return x;
      }
    }
    return {};
  }

  auto position(auto&& pred) -> option::Option<usize> {
    for (usize idx = 0UL; auto x = this->next(); ++idx) {
      if (pred(*x)) {
        return idx;
      }
    }

    return {};
  }

  auto rposition(auto&& pred) -> option::Option<usize> {
    for (auto idx = this->len() - 1; auto x = this->next_back(); --idx) {
      if (pred(*x)) {
        return idx;
      }
    }
    return {};
  }

  auto all(auto&& f) -> bool {
    return !this->position([&](auto& x) { return !f(x); });
  }

  auto any(auto&& f) -> bool {
    return this->position(f).is_some();
  }

  auto count(auto&& f) -> usize {
    auto accum = usize{0};
    this->for_each([&](auto& x) mutable {
      if (f(x)) {
        accum += 1;
      }
    });
    return accum;
  }

  auto min() -> option::Option<Item> {
    return this->reduce([](auto&& a, auto&& b) -> auto& { return a < b ? a : b; });
  }

  auto max() -> option::Option<Item> {
    return this->reduce([](auto&& a, auto&& b) -> auto& { return a > b ? a : b; });
  }

  auto min_by_key(auto&& f) -> option::Option<Item> {
    return this->reduce([&](auto&& a, auto&& b) -> auto& { return f(a) < f(b) ? a : b; });
  }

  auto max_by_key(auto&& f) -> option::Option<Item> {
    return this->reduce([&](auto&& a, auto&& b) -> auto& { return f(a) > f(b) ? a : b; });
  }

  template <class S = decltype(+declval<Item>())>
  auto sum(S init = 0) -> S {
    return this->fold(init, [](const auto& a, const auto& b) { return a + b; });
  }

  template <class S = decltype(+declval<Item>())>
  auto product(S init = 1) -> S {
    return this->fold(init, [](const auto& a, const auto& b) { return a * b; });
  }

  template <class B>
  auto collect() -> B {
    return B::from_iter(static_cast<I&&>(*this));
  }

  auto rev() -> Rev<I> {
    return Rev{static_cast<I&&>(*this)};
  }

  template <class F>
  auto map(F f) -> Map<I, F> {
    return Map{static_cast<I&&>(*this), static_cast<F&&>(f)};
  }
};

template <class I>
struct Rev {
  using Item = typename I::Item;
  I _iter;

 public:
  auto len() const -> usize {
    return _iter.len();
  }

  auto next() -> option::Option<Item> {
    return _iter.next_back();
  }

  auto next_back() -> option::Option<Item> {
    return _iter.next();
  }

  auto operator->() const -> const iter::Iterator<Rev>* {
    return this;
  }

  auto operator->() -> iter::Iterator<Rev>* {
    return this;
  }
};

template <class I, class F>
struct Map {
  using Item = expr_t<F(typename I::Item)>;

  I _iter;
  F _func;

 public:
  auto len() const -> usize {
    return _iter.len();
  }

  auto next() -> option::Option<Item> {
    return _iter.next().map(_func);
  }

  auto next_back() -> option::Option<Item> {
    return _iter.next_back().map(_func);
  }

  auto operator->() const -> const iter::Iterator<Map>* {
    return this;
  }

  auto operator->() -> iter::Iterator<Map>* {
    return this;
  }
};

template <class A, class B>
struct Zip {
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

  auto operator->() const -> const iter::Iterator<Zip>* {
    return this;
  }

  auto operator->() -> iter::Iterator<Zip>* {
    return this;
  }
};

template <class A, class B>
auto zip(A a, B b) -> Zip<A, B> {
  return Zip<A, B>{static_cast<A&&>(a), static_cast<B&&>(b)};
}

}  // namespace sfc::iter
