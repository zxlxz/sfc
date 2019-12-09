#pragma once

#include "rc/core/option.h"
#include "rc/core/ptr.h"

namespace rc::iter {

template <class Self>
struct Rev;

template <class Self>
struct Iterator {
  template <class T>
  auto find(const T& val) noexcept -> Option<usize> {
    auto& self = *ptr::cast<Self>(this);

    for (usize idx = 0;; ++idx) {
      const auto res = self.next();
      if (res.is_none()) break;
      if (res.as_some() == val) return {idx};
    }
    return {};
  }

  template <class F>
  auto find_if(F&& fun) noexcept -> Option<usize> {
    auto& self = *ptr::cast<Self>(this);

    for (usize idx = 0;; ++idx) {
      const auto res = self.next();
      if (res.is_none()) break;
      if (fun(res.as_some())) return {idx};
    }
    return {};
  }

  template <class T>
  auto count(const T& val) noexcept -> usize {
    auto& self = *ptr::cast<Self>(this);

    for (usize cnt = 0;; ++cnt) {
      const auto res = self.next();
      if (res.is_none()) return cnt;
      if (res.as_some() == val) {
        cnt += 1;
      }
    }
    return 0;
  }

  template <class F>
  auto count_if(F&& fun) noexcept -> usize {
    auto& self = *ptr::cast<Self>(this);

    for (usize cnt = 0;;) {
      const auto res = self.next();
      if (res.is_none()) return cnt;
      if (fun(res.as_some())) ++cnt;
    }
    return 0;
  }

  template <class T>
  auto contains(const T& val) noexcept -> bool {
    auto& self = *ptr::cast<Self>(this);

    while (true) {
      const auto res = self.next();
      if (res.is_none()) return false;
      if (res.as_some() == val) return true;
    }
    return false;
  }

  template <class F>
  auto contains_if(F&& fun) noexcept -> bool {
    auto& self = *ptr::cast<Self>(this);

    while (true) {
      const auto res = self.next();
      if (res.is_none()) return false;
      if (fun(res.as_some())) return true;
    }
    return false;
  }

  template <class T>
  auto rfind(const T& val) noexcept -> Option<usize> {
    auto& self = *ptr::cast<Self>(this);

    const usize n = self.len();
    for (usize idx = 0;; ++idx) {
      const auto res = self.next_back();
      if (res.as_some() == val) return {n - idx};
      if (res.is_none()) break;
    }
    return {};
  }

  template <class F>
  auto rfind_if(F&& fun) noexcept -> Option<usize> {
    auto& self = *ptr::cast<Self>(this);

    const usize n = self.len();
    for (usize idx = 0;; ++idx) {
      const auto res = self.next();
      if (res.is_none()) break;
      if (fun(res.as_some())) return {n - idx};
    }
    return {};
  }

  auto rev() && -> Rev<Self>;
};

template <class Iter>
struct Rev : Iterator<Rev<Iter>> {
  using Item = typename Iter::Item;

  Iter _inner;

  auto next() -> Option<Item> { return _inner.next_back(); }
};

template <class Self>
inline auto Iterator<Self>::rev() && -> Rev<Self> {
  auto& self = *ptr::cast<Self>(this);
  return Rev<Self>{self};
}

}  // namespace rc::iter
