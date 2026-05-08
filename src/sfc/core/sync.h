#pragma once

#include "sfc/core/mod.h"

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
    return __atomic_load_n(&_val, int(order));
#endif
  }

  void store(T val, Ordering order = Ordering::SeqCst) noexcept {
#if defined(__INTELLISENSE__) || defined(__clang_analyzer__)
    _val = val;
#else
    return __atomic_store_n(&_val, val, int(order));
#endif
  }

  auto exchange(T val, Ordering order = Ordering::SeqCst) noexcept -> T {
#if defined(__INTELLISENSE__) || defined(__clang_analyzer__)
    return mem::replace(_val, val);
#else
    return __atomic_exchange_n(&_val, val, int(order));
#endif
  }

  auto fetch_add(T val, Ordering order = Ordering::SeqCst) noexcept -> T {
#if defined(__INTELLISENSE__) || defined(__clang_analyzer__)
    return mem::replace(_val, _val + val);
#else
    return __atomic_fetch_add(&_val, val, int(order));
#endif
  }

  auto fetch_sub(T val, Ordering order = Ordering::SeqCst) noexcept -> T {
#if defined(__INTELLISENSE__) || defined(__clang_analyzer__)
    return mem::replace(_val, _val - val);
#else
    return __atomic_fetch_sub(&_val, val, int(order));
#endif
  }

  auto compare_exchange(T current, T new_val, Ordering success, Ordering failure) noexcept -> bool {
#if defined(__INTELLISENSE__) || defined(__clang_analyzer__)
    if (_val != current) return false;
    _val = new_val;
    return true;
#else
    return __atomic_compare_exchange_n(&_val, &current, new_val, 0, int(success), int(failure));
#endif
  }
};

}  // namespace sfc::sync
