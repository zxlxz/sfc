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
  T _val;

 public:
  auto load(Ordering order = Ordering::SeqCst) const noexcept -> T {
#if defined(__INTELLISENSE__) || defined(__clang_analyzer__)
    return _val;
#else
    return __atomic_load_n(static_cast<const volatile T*>(&_val), static_cast<int>(order));
#endif
  }

  void store(T val, Ordering order = Ordering::SeqCst) noexcept {
#if defined(__INTELLISENSE__) || defined(__clang_analyzer__)
    _val = val;
#else
    return __atomic_store_n(static_cast<volatile T*>(&_val), val, static_cast<int>(order));
#endif
  }

  auto exchange(T val, Ordering order = Ordering::SeqCst) noexcept -> T {
#if defined(__INTELLISENSE__) || defined(__clang_analyzer__)
    return mem::replace(_val, val);
#else
    return __atomic_exchange_n(static_cast<volatile T*>(&_val), val, static_cast<int>(order));
#endif
  }

  auto fetch_add(T val, Ordering order = Ordering::SeqCst) noexcept -> T {
#if defined(__INTELLISENSE__) || defined(__clang_analyzer__)
    return mem::replace(_val, _val + val);
#else
    return __atomic_fetch_add(static_cast<volatile T*>(&_val), val, static_cast<int>(order));
#endif
  }

  auto fetch_sub(T val, Ordering order = Ordering::SeqCst) noexcept -> T {
#if defined(__INTELLISENSE__) || defined(__clang_analyzer__)
    return mem::replace(_val, _val - val);
#else
    return __atomic_fetch_sub(static_cast<volatile T*>(&_val), val, static_cast<int>(order));
#endif
  }
};

}  // namespace sfc::sync
