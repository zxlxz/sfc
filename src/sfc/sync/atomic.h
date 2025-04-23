#pragma once

#include "sfc/core.h"

namespace sfc::sync {

enum class Ordering {
  Relaxed = __ATOMIC_RELAXED,
  Acquire = __ATOMIC_ACQUIRE,
  Release = __ATOMIC_RELEASE,
  AcqRel = __ATOMIC_ACQ_REL,
  SeqCst = __ATOMIC_SEQ_CST,
};

template <class T>
class Atomic {
  T _val;

 public:
  explicit Atomic(T val) : _val{val} {}

  void operator=(T val) noexcept {
    this->store(val, Ordering::SeqCst);
  }

  void operator+=(T val) {
    this->fetch_add(val);
  }

  void operator-=(T val) {
    this->fetch_sub(val);
  }

  auto load(Ordering order = Ordering::SeqCst) const noexcept {
    return __atomic_load_n(&_val, static_cast<int>(order));
  }

  void store(T val, Ordering order = Ordering::SeqCst) noexcept {
    __atomic_store(&_val, &val, static_cast<int>(order));
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
