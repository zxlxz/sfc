#pragma once

#include "sfc/core.h"

namespace sfc::sync {

enum class Ordering {
  Relaxed,
  Consume,
  Acquire,
  Release,
  AcqRel,
  SeqCst,
};

template <class T>
struct Atomic {
  static_assert(!__is_class(T) && !__is_union(T));

  T _val;

 public:
  auto load(Ordering order = Ordering::SeqCst) const noexcept -> T {
    return __atomic_load_n(const_cast<T*>(&_val), static_cast<int>(order));
  }

  void store(T val, Ordering order = Ordering::SeqCst) noexcept {
    return __atomic_store_n(&_val, val, static_cast<int>(order));
  }

  auto exchange(T val, Ordering order = Ordering::SeqCst) noexcept -> T {
    return __atomic_exchange_n(&_val, val, static_cast<int>(order));
  }

  auto fetch_add(T val, Ordering order = Ordering::SeqCst) noexcept -> T {
    return __atomic_fetch_add(&_val, val, static_cast<int>(order));
  }

  auto fetch_sub(T val, Ordering order = Ordering::SeqCst) noexcept -> T {
    return __atomic_fetch_sub(&_val, val, static_cast<int>(order));
  }
};

}  // namespace sfc::sync
