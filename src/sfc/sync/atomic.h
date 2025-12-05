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
  static_assert(trait::int_<T> || trait::enum_<T> || trait::ptr_<T>);
  T _val;

 public:
  template <Ordering order = Ordering::SeqCst>
  auto load() const noexcept -> T {
    return __atomic_load_n(const_cast<T*>(&_val), static_cast<int>(order));
  }

  template <Ordering order = Ordering::SeqCst>
  void store(T val) noexcept {
    return __atomic_store_n(&_val, val, static_cast<int>(order));
  }

  template <Ordering order = Ordering::SeqCst>
  auto exchange(T val) noexcept -> T {
    return __atomic_exchange_n(&_val, val, static_cast<int>(order));
  }

  template <Ordering order = Ordering::SeqCst>
  auto fetch_add(T val) noexcept -> T {
    static_assert(trait::int_<T>);
    return __atomic_fetch_add(&_val, val, static_cast<int>(order));
  }

  template <Ordering order = Ordering::SeqCst>
  auto fetch_sub(T val) noexcept -> T {
    static_assert(trait::int_<T>);
    return __atomic_fetch_sub(&_val, val, static_cast<int>(order));
  }
};

}  // namespace sfc::sync
