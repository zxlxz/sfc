#pragma once

#include "rc/alloc.h"
#include "rc/core/atomic.h"

namespace rc::sync {

using atomic::Ordering;
using atomic::Atomic;

template<class T>
struct Arc {
  struct Inner {
    Atomic<u32> _cnt;
    T _data;
  };
  Inner* _p;

  Arc(Arc&& other) noexcept : _p{other._p} {
    other.forget();
  }
  
  ~Arc() {
    if (_p == nullptr) return;
    if (_p->_cnt.fetch_sub(1, Ordering::Release) == 1) {
      atomic::fence();
    }
  }

  void forget() { _p = nullptr; }

  static auto from_ptr(Inner* p) noexcept -> Arc { return Arc{p}; }

  static auto from_raw(T* p) noexcept -> Arc {
    const auto offset = __builtin_offsetof(Inner, _data);
    const auto fake_p = ptr::cast<Inner>(ptr::cast<u8>(p) - offset);
    return Arc{fake_p};
  }
  
  static auto create(T val) -> Arc {
    auto x = Box{Inner{1, rc::move(val)}};
    return Arc::from_ptr(rc::move(x).into_raw());
  }

  auto into_raw() && -> T* { return _p->_data; }
};

}
