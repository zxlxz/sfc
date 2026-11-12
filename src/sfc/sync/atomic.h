#pragma once

#include "sfc/core/mem.h"

namespace sfc::sync {

enum class Ordering {
  Relaxed = 0,  // no ordering constraints
  Consume = 1,  // load
  Acquire = 2,  // load
  Release = 3,  // store
  AcqRel = 4,   // RMW (read-modify-write)
  SeqCst = 5,   // sequentially consistent
};

template <class T>
struct Atomic {
  T _val{0};

 public:
  template <Ordering order = Ordering::SeqCst>
  auto load() const noexcept -> T {
    static_assert(order == Ordering::Relaxed || order == Ordering::Acquire || order == Ordering::SeqCst);
    return __atomic_load_n(&_val, int(order));
  }

  template <Ordering order = Ordering::SeqCst>
  void store(T val) noexcept {
    static_assert(order == Ordering::Relaxed || order == Ordering::Release || order == Ordering::SeqCst);
    return __atomic_store_n(&_val, val, int(order));
  }

  template <Ordering order = Ordering::SeqCst>
  auto exchange(T val) noexcept -> T {
    static_assert(order != Ordering::Consume);
    return __atomic_exchange_n(&_val, val, int(order));
  }

  template <Ordering order = Ordering::SeqCst>
  auto fetch_add(T val) noexcept -> T {
    static_assert(order != Ordering::Consume);
    return __atomic_fetch_add(&_val, val, int(order));
  }

  template <Ordering order = Ordering::SeqCst>
  auto fetch_sub(T val) noexcept -> T {
    static_assert(order != Ordering::Consume);
    return __atomic_fetch_sub(&_val, val, int(order));
  }

  template <Ordering success = Ordering::SeqCst, Ordering failure = Ordering::Acquire>
  auto compare_exchange(T current, T new_val) noexcept -> bool {
    static_assert(success != Ordering::Consume);
    static_assert(failure != Ordering::Consume && failure != Ordering::Release && failure != Ordering::AcqRel);
    return __atomic_compare_exchange_n(&_val, &current, new_val, 0, int(success), int(failure));
  }

  template <Ordering success = Ordering::SeqCst, Ordering failure = Ordering::Acquire>
  auto compare_exchange_weak(T current, T new_val) noexcept -> bool {
    static_assert(success != Ordering::Consume);
    static_assert(failure != Ordering::Consume && failure != Ordering::Release && failure != Ordering::AcqRel);
    return __atomic_compare_exchange_n(&_val, &current, new_val, 1, int(success), int(failure));
  }
};

template <Ordering order = Ordering::SeqCst>
inline void fence() noexcept {
  static_assert(order != Ordering::Consume);
  __atomic_thread_fence(int(order));
}

}  // namespace sfc::sync
