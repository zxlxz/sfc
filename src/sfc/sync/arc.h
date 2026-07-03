#pragma once

#include "sfc/alloc/boxed.h"

namespace sfc::sync {

template <class T>
class [[nodiscard]] Arc {
  struct Inn {
    sync::Atomic<int> _cnt;
    T _val;

   public:
    explicit Inn(auto&&... args) noexcept : _cnt{1}, _val{(decltype(args)&&)(args)...} {}

    auto inc_count() noexcept -> int {
      return _cnt.fetch_add(1, sync::Ordering::Relaxed);
    }

    auto dec_count() noexcept -> int {
      return _cnt.fetch_sub(1, sync::Ordering::AcqRel);
    }
  };

  Box<Inn> _inn{};

 public:
  Arc() noexcept = default;

  ~Arc() noexcept {
    const auto p = _inn.ptr();
    if (p && p->dec_count() > 1) {
      // don't need to drop, just forget to avoid double drop
      mem::forget(_inn);
    }
  }

  Arc(Arc&& other) noexcept = default;

  Arc& operator=(Arc&& other) noexcept = default;

  template <class... U>
  static auto xnew(U&&... args) -> Arc {
    auto res = Arc{};
    res._inn = Box<Inn>::xnew(mem::move<U>(args)...);
    return res;
  }

  auto as_ptr() const noexcept -> T* {
    const auto p = _inn.ptr();
    return p ? &p->_val : nullptr;
  }

 public:
  // trait: Deref<const T*>
  auto operator->() const noexcept -> const T* {
    return &_inn->_val;
  }

  // trait: Deref<T*>
  auto operator->() noexcept -> T* {
    return &_inn->_val;
  }

  // trait: Deref<const T&>
  auto operator*() const noexcept -> const T& {
    return _inn->_val;
  }

  // trait: Deref<T&>
  auto operator*() noexcept -> T& {
    return _inn->_val;
  }

  // trait: Clone
  auto clone() const noexcept -> Arc {
    auto res = Arc{};
    if (auto ptr = _inn.ptr()) {
      ptr->inc_count();
      res._inn = Box<Inn>::from_raw(ptr);
    }
    return res;
  }
};

}  // namespace sfc::sync

namespace sfc {
using sync::Arc;
}  // namespace sfc
