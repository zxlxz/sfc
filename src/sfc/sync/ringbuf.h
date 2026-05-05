#pragma once

#include "sfc/sync/atomic.h"
#include "sfc/alloc/alloc.h"

namespace sfc::sync::ringbuf {

template <class T>
struct Cell {
  Atomic<usize> _seq;
  union {
    T _val;
  };

 public:
  Cell(usize seq) noexcept : _seq{seq} {}

  ~Cell() noexcept {}

  auto seq() const noexcept -> usize {
    return _seq.load(Ordering::Acquire);
  }

  auto take(usize seq) noexcept -> T {
    auto val = ptr::read(&_val);
    _seq.store(seq, Ordering::Release);
    return val;
  }

  void set(usize seq, T val) noexcept {
    ptr::write(&_val, static_cast<T&&>(val));
    _seq.store(seq, Ordering::Release);
  }
};

template <class T, class A = alloc::Global>
struct RawBuf {
  using Cell = ringbuf::Cell<T>;
  Cell* _ptr{nullptr};
  usize _cap{0};
  A _a;

 public:
  RawBuf() noexcept {}

  ~RawBuf() {
    if (_ptr == nullptr) return;
    const auto layout = Layout::array<Cell>(_cap);
    _a.dealloc(_ptr, layout);
  }

  RawBuf(RawBuf&& other) noexcept
      : _ptr{mem::take(other._ptr)}, _cap{mem::take(other._cap)}, _a{mem::move(other._a)} {}

  RawBuf& operator=(RawBuf&& other) noexcept {
    if (this != &other) {
      mem::swap(_ptr, other._ptr);
      mem::swap(_cap, other._cap);
      mem::swap(_a, other._a);
    }
    return *this;
  }

  static auto with_capacity(usize min_cap, A a = {}) noexcept -> RawBuf {
    const auto cap = num::next_power_of_two(min_cap);
    auto res = RawBuf{};
    if (cap == 0) return res;

    const auto layout = Layout::array<Cell>(cap);
    auto ptr = static_cast<Cell*>(a.alloc(layout));
    for (auto i = 0UL; i < cap; ++i) {
      new (ptr + i) Cell{i};
    }

    res._ptr = ptr;
    res._cap = cap;
    res._a = a;
    return res;
  }

  auto operator[](usize idx) noexcept -> Cell& {
    return _ptr[idx & (_cap - 1)];
  }
};

template <class T, class A = alloc::Global>
class RingBuf {
  using Inn = ringbuf::RawBuf<T, A>;

  Atomic<usize> _head{0};
  Atomic<usize> _tail{0};
  Inn _inn;

 public:
  RingBuf() noexcept {}

  ~RingBuf() noexcept {
    this->clear();
  }

  RingBuf(RingBuf&& other) noexcept = default;
  RingBuf& operator=(RingBuf&& other) noexcept = default;

  static auto with_capacity(usize capacity, A alloc = {}) -> RingBuf {
    auto res = RingBuf{};
    res._inn = Inn::with_capacity(capacity, alloc);
    return res;
  }

 public:
  auto push(T val) noexcept -> Result<void, T> {
    if (this->try_push(val)) {
      return Ok{};
    }
    return Err{static_cast<T&&>(val)};
  }

  auto try_push(T& val) noexcept -> bool {
    if (_inn._cap == 0) {
      return false;
    }

    while (true) {
      const auto head = _head.load(Ordering::Relaxed);
      auto& cell = _inn[head];
      const auto seq = cell.seq();
      if (seq < head) {  // full
        return false;
      }
      if (seq > head) {  // stale
        continue;
      }
      if (_head.compare_exchange(head, head + 1, Ordering::Relaxed, Ordering::Relaxed)) {
        cell.set(head + 1, static_cast<T&&>(val));
        return true;
      }
    }
  }

  auto pop() noexcept -> Option<T> {
    if (_inn._cap == 0) {
      return {};
    }

    while (true) {
      auto tail = _tail.load(Ordering::Relaxed);
      auto& cell = _inn[tail];
      const auto seq = cell.seq();
      if (seq < tail + 1) {  // empty
        return {};
      }
      if (seq > tail + 1) {  // stale
        continue;
      }
      if (_tail.compare_exchange(tail, tail + 1, Ordering::Relaxed, Ordering::Relaxed)) {
        auto val = cell.take(tail + _inn._cap);
        return val;
      }
    }
  }

  void clear() {
    while (true) {
      if (!this->pop()) break;
    }
  }
};

}  // namespace sfc::sync::ringbuf

namespace sfc::sync {
using ringbuf::RingBuf;
}
