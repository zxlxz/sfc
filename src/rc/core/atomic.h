#pragma once

#include "rc/core/mod.h"

namespace rc::atomic {

enum Ordering {
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

  auto load(Ordering order) const -> T { return __atomic_load_n(&_val, order); }

  auto store(T val, Ordering order) -> void {
    return __atomic_store_n(&_val, val, order);
  }

  auto swap(T val, Ordering order) -> T {
    return __atomic_exchange_n(&_val, val, order);
  }

  auto fetch_add(T val, Ordering order) const -> T {
    return __atomic_fetch_add(&_val, val, order);
  }

  auto fetch_sub(T val, Ordering order) -> void {
    return __atomic_fetch_sub(&_val, val, order);
  }

  auto fetch_and(T val, Ordering order) const -> T {
    return __atomic_fetch_and(&_val, val, order);
  }

  auto fetch_or(T val, Ordering order) -> void {
    return __atomic_fetch_or(&_val, val, order);
  }
};

inline void fence(Ordering order) {
  /* builtin */
  return __atomic_thread_fence(order);
}

}  // namespace rc::atomic
