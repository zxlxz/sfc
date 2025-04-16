#pragma once

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

 public:
  auto nth(usize n) -> Option<Item> {
    for (auto i = 0UL; i < n; ++i) {
      this->next();
    }
    return this->next();
  }

  void for_each(auto&& f) {
    for (; auto x = this->next();) {
      f(x.get_unchecked_mut());
    }
  }

  void for_each_idx(auto&& f) {
    for (auto i = 0UL; auto x = this->next(); ++i) {
      f(i, x.get_unchecked_mut());
    }
  }

  template <class B>
  auto fold(B init, auto&& f) -> B {
    using U = trait::pack_element_t<trait::isRef<B>, B, mem::Ref<B>>;

    auto accum = U{static_cast<B&&>(init)};
    for (; auto x = this->next();) {
      accum = f(accum, x.get_unchecked_mut());
    }
    return accum;
  }

  auto reduce(auto&& f) -> Item {
    auto first = this->next();
    panicking::assert_fmt(first, "iter::Iterator::reduce: empty iter");

    return this->fold<Item>(mem::move(first).unwrap(), f);
  }

  auto find(auto&& pred) -> Option<Item> {
    for (; auto x = this->next();) {
      if (pred(x.get_unchecked_mut())) {
        return x;
      }
    }
    return {};
  }

  auto rfind(auto&& pred) -> Option<Item> {
    for (; auto x = this->next_back();) {
      if (pred(x.get_unchecked_mut())) {
        return x;
      }
    }
    return {};
  }

  auto position(auto&& pred) -> Option<usize> {
    for (auto idx = 0UL; auto x = this->next(); ++idx) {
      if (pred(x.get_unchecked_mut())) {
        return idx;
      }
    }
    return {};
  }

  auto rposition(auto&& pred) -> Option<usize> {
    for (auto idx = this->len() - 1; auto x = this->next_back(); --idx) {
      if (pred(x.get_unchecked_mut())) {
        return idx;
      }
    }
    return {};
  }

  auto all(auto&& f) -> bool {
    auto res = this->find([&](auto& x) { return !f(x); });
    return !bool(res);
  }

  auto any(auto&& f) -> bool {
    auto res = this->find(f);
    return bool(res);
  }

  auto count(auto&& f) -> usize {
    auto accum = 0UL;
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
    auto init = decltype(declval<Item>() + declval<Item>()){0};
    return this->fold(init, [](auto&& a, auto&& b) { return a + b; });
  }

  auto product() {
    auto init = decltype(declval<Item>() * declval<Item>()){1};
    return this->fold(init, [](auto&& a, auto&& b) { return a * b; });
  }

  template <class B>
  auto collect() -> B {
    return B::from_iter(*this);
  }

  auto rev() -> Rev<I>;

  template <class F>
  auto map(F func) -> Map<I, F>;

  template <class P>
  auto filter(P pred) -> Filter<I, P>;
};

template <class I>
struct Cursor {
  using Item = typename I::Item;
  struct End {};

  Iterator<I>&         _iter;
  option::Option<Item> _item;

 public:
  auto operator!=(End) const -> bool {
    return bool(_item);
  }

  void operator++() {
    _item = _iter.next();
  }

  auto operator*() -> Item& {
    return _item.get_unchecked_mut();
  }
};

template <class I>
auto begin(Iterator<I>& iter) -> Cursor<I> {
  return {iter, iter.next()};
}

template <class I>
auto end(Iterator<I>&) -> Cursor<I>::End {
  return {};
}

}  // namespace sfc::iter
