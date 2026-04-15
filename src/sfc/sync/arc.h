#pragma once

#include "sfc/alloc/boxed.h"
#include "sfc/sync/atomic.h"

namespace sfc::sync {

template <class T>
class [[nodiscard]] Arc {
  struct Inn;
  Box<Inn> _inn{};

 public:
  Arc() noexcept = default;

  ~Arc() noexcept {
    const auto p = _inn.ptr();
    if (p && p->dec_count() != 1) {
      (void)mem::move(_inn).into_raw();  // prevent dropping
    }
  }

  Arc(Arc&& other) noexcept = default;

  Arc& operator=(Arc&& other) noexcept = default;

  static auto xnew(auto&&... args) -> Arc {
    auto res = Arc{};
    res._inn = Box<Inn>::xnew(1, static_cast<decltype(args)&&>(args)...);
    return res;
  }

  auto as_ptr() const noexcept -> T* {
    const auto p = _inn.ptr();
    return p ? &p->_val : nullptr;
  }

 public:
  // trait: Deref<const T*>
  [[gnu::always_inline]] auto operator->() const noexcept -> const T* {
    return &_inn->_val;
  }

  // trait: Deref<T*>
  [[gnu::always_inline]] auto operator->() noexcept -> T* {
    return &_inn->_val;
  }

  // trait: Deref<const T&>
  [[gnu::always_inline]] auto operator*() const noexcept -> const T& {
    return _inn->_val;
  }

  // trait: Deref<T&>
  [[gnu::always_inline]] auto operator*() noexcept -> T& {
    return _inn->_val;
  }

  // trait: Clone
  auto clone() const noexcept -> Arc {
    auto ptr = _inn.ptr();
    auto res = Arc{};
    if (ptr) {
      ptr->inc_count();
      res._inn = Box<Inn>::from_raw(ptr);
    }
    return res;
  }
};

template <class T>
struct Arc<T>::Inn {
  sync::Atomic<int> _cnt{1};
  T _val;

  auto inc_count() noexcept -> int {
    return _cnt.fetch_add(1, sync::Ordering::AcqRel);
  }

  auto dec_count() noexcept -> int {
    return _cnt.fetch_sub(1, sync::Ordering::AcqRel);
  }
};

}  // namespace sfc::sync

namespace sfc {
using sync::Arc;
}  // namespace sfc
