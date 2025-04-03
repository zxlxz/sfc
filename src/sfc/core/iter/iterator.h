#pragma once

#include "sfc/core/option.h"
#include "sfc/core/tuple.h"

namespace sfc::iter {

template <class I, class T>
struct Iterator {
  using Item = T;

 public:
  auto nth(usize n) -> Option<Item> {
    auto& self = static_cast<I&>(*this);
    for (auto i = 0U; i < n; ++i) {
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
    for (auto i = 0U; auto x = self.next(); ++i) {
      f(i, x.get_unchecked_mut());
    }
  }

  template <class B>
  auto fold(B init, auto&& f) -> B {
    auto& self = static_cast<I&>(*this);

    auto accum = init;
    for (; auto x = self.next();) {
      accum = f(accum, x.get_unchecked_mut());
    }
    return accum;
  }

  template <trait::isRef B>
  auto fold(B init, auto&& f) -> B {
    auto& self = static_cast<I&>(*this);

    auto accum_ptr = &init;
    for (; auto x = self.next();) {
      accum_ptr = &f(*accum_ptr, x.get_unchecked_mut());
    }
    return *accum_ptr;
  }

  auto reduce(auto&& f) -> Item {
    auto& self = static_cast<I&>(*this);
    auto  first = self.next();
    assert_fmt(first, "iter::Iterator::reduce: empty iter");

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
    for (auto idx = 0UL; auto x = self.next(); ++idx) {
      if (pred(x.get_unchecked_mut())) {
        return idx;
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
    auto& self = static_cast<I&>(*this);

    auto res = this->find([&](auto& x) {
      return !f(x);
    });
    return !bool(res);
  }

  auto any(auto&& f) -> bool {
    auto res = this->find(f);
    return bool(res);
  }

  auto count(auto&& f) -> usize {
    auto& self = static_cast<I&>(*this);

    auto accum = 0UL;
    this->for_each([&](auto& x) mutable {
      f(x) ? accum += 1 : void();
    });
    return accum;
  }

  auto min() -> Item {
    return this->reduce([](auto&& a, auto&& b) -> auto& {
      return a < b ? a : b;
    });
  }

  auto max() -> Item {
    return this->reduce([](auto&& a, auto&& b) -> auto& {
      return a > b ? a : b;
    });
  }

  auto min_by_key(auto&& f) {
    return this->reduce([&](auto&& a, auto&& b) -> auto& {
      return f(a) < f(b) ? a : b;
    });
  }

  auto max_by_key(auto&& f) {
    return this->reduce([&](auto&& a, auto&& b) -> auto& {
      return f(a) > f(b) ? a : b;
    });
  }

  auto sum() {
    auto& self = static_cast<I&>(*this);
    auto  init = decltype(*self.next() + *self.next()){0};
    return this->fold(init, [](auto&& a, auto&& b) {
      return a + b;
    });
  }

  auto product() {
    auto& self = static_cast<I&>(*this);
    auto  init = decltype(*self.next() + *self.next()){1};
    return this->fold(init, [](auto&& a, auto&& b) {
      return a * b;
    });
  }

  auto rev();

  auto enumerate();

  template <class F>
  auto map(F func);

  template <class P>
  auto filter(P pred);
};

}  // namespace sfc::iter
