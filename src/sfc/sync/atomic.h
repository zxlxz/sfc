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

#if defined(_MSC_VER) && !defined(__clang__)
// clang-format off
template <class T>auto __atomic_load_n    (T* ptr, int) -> T;
template <class T>void __atomic_store_n   (T* ptr, T val, int);
template <class T>auto __atomic_exchange_n(T* ptr, T val, int) -> T;
template <class T>auto __atomic_fetch_add (T* ptr, T val, int) -> T;
template <class T>auto __atomic_fetch_sub (T* ptr, T val, int) -> T;
// clang-format on
#endif

template <class T>
class Atomic {
  T _val;

 public:
  explicit Atomic(T val) noexcept : _val{val} {}

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
    return __atomic_fetch_add(&_val, val, static_cast<int>(order));
  }

  template <Ordering order = Ordering::SeqCst>
  auto fetch_sub(T val) noexcept -> T {
    return __atomic_fetch_sub(&_val, val, static_cast<int>(order));
  }
};

}  // namespace sfc::sync
