#pragma once

#include "sfc/alloc/alloc.h"

namespace sfc::sync {

template <class T>
class Queue {
  struct Cell {
    Atomic<usize> _seq;
    T _val;

    auto seq() const noexcept -> usize {
      return _seq.load(Ordering::Acquire);
    }

    void init(usize idx) noexcept {
      _seq.store(idx, Ordering::Relaxed);
    }

    void set(usize idx, T val) noexcept {
      ptr::write(&_val, mem::move(val));
      _seq.store(idx, Ordering::Release);
    }

    auto take(usize idx) noexcept -> T {
      const auto val = ptr::read(&_val);
      _seq.store(idx, Ordering::Release);
      return val;
    }
  };

  using Buf = alloc::RawBuf<Cell>;
  Atomic<usize> _head{0};
  Atomic<usize> _tail{0};
  Buf _buf;

 public:
  Queue() noexcept {}

  ~Queue() noexcept {
    this->clear();
  }

  Queue(Queue&& other) noexcept = default;
  Queue& operator=(Queue&& other) noexcept = default;

  static auto with_capacity(usize capacity, Allocator& alloc = alloc::Global::instance()) -> Queue {
    auto res = Queue{};
    res._buf = Buf::with_capacity(capacity, alloc);

    // init
    for (auto i = 0U; i < capacity; ++i) {
      res._buf[i].init(i);
    }
    return res;
  }

 public:
  auto is_empty() const noexcept -> bool {
    const auto head = _head.load(Ordering::Acquire);
    const auto tail = _tail.load(Ordering::Acquire);
    return head == tail;
  }

  auto is_full() const noexcept -> bool {
    const auto head = _head.load(Ordering::Acquire);
    const auto tail = _tail.load(Ordering::Acquire);
    return head - tail >= _buf.cap();
  }

  auto push(T val) noexcept -> Result<void, T> {
    if (_buf.cap() == 0) {
      return Result<void, T>{mem::move(val)};
    }

    while (true) {
      const auto head = _head.load(Ordering::Acquire);
      auto& cell = _buf[head % _buf.cap()];

      const auto seq = cell.seq();
      if (seq < head) break;
      if (seq > head) continue;

      if (_head.compare_exchange(head, head + 1, Ordering::Release, Ordering::Acquire)) {
        cell.set(head + 1, mem::move(val));
        return Result<void, T>{};
      }
    }
    return Result<void, T>{mem::move(val)};
  }

  auto pop() noexcept -> Option<T> {
    if (_buf.cap() == 0) {
      return Option<T>{};
    }

    while (true) {
      auto tail = _tail.load(Ordering::Acquire);
      auto& cell = _buf[tail % _buf.cap()];
      const auto seq = cell.seq();
      if (seq < tail + 1) break;
      if (seq > tail + 1) continue;

      if (_tail.compare_exchange(tail, tail + 1, Ordering::Release, Ordering::Acquire)) {
        return Option<T>{cell.take(tail + _buf.cap())};
      }
    }
    return Option<T>{};
  }

  void clear() {
    while (true) {
      if (!this->pop()) break;
    }
  }
};

}  // namespace sfc::sync
