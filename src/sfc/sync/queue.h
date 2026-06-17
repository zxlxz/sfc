#pragma once

#include "sfc/alloc/alloc.h"

namespace sfc::sync {

template <class T, class A = alloc::Global>
class Queue {
  struct Cell;
  using Buf = alloc::RawBuf<Cell, A>;

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

  static auto with_capacity(usize capacity, A alloc = {}) -> Queue {
    auto res = Queue{};
    res._buf = Buf::with_capacity(capacity, alloc);
    res.init();
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
    if (this->try_push(val)) {
      return {};
    }
    return {mem::move(val)};
  }

  auto try_push(T& val) noexcept -> bool {
    const auto idx = this->try_push_idx();
    if (!idx) {
      return false;
    }

    ptr::write(&_buf[*idx]._val, mem::move(val));
    return true;
  }

  auto pop() noexcept -> Option<T> {
    const auto idx = this->pop_idx();
    if (!idx) {
      return {};
    }

    return ptr::read(&_buf[*idx]._val);
  }

  void clear() {
    while (true) {
      if (!this->pop()) break;
    }
  }

 private:
  struct Cell {
    Atomic<usize> _seq;
    T _val;
  };

  void init() noexcept {
    const auto n = _buf.cap();
    for (auto i = 0UL; i < n; ++i) {
      _buf[i]._seq.store(i, Ordering::Relaxed);
    }
  }

  auto try_push_idx() noexcept -> Option<usize> {
    if (_buf.cap() == 0) {
      return {};
    }

    while (true) {
      const auto head = _head.load(Ordering::Acquire);
      const auto offset = head % _buf.cap();

      auto& cell = _buf[offset]._seq;
      const auto seq = cell.load(Ordering::Acquire);
      if (seq < head) break;
      if (seq > head) continue;

      if (_head.compare_exchange(head, head + 1, Ordering::Release, Ordering::Acquire)) {
        cell.store(head + 1, Ordering::Release);
        return {offset};
      }
    }
    return {};
  }

  auto pop_idx() noexcept -> Option<usize> {
    if (_buf.cap() == 0) {
      return {};
    }

    while (true) {
      auto tail = _tail.load(Ordering::Acquire);
      const auto offset = tail % _buf.cap();

      auto& cell = _buf[offset]._seq;
      const auto seq = cell.load(Ordering::Acquire);
      if (seq < tail + 1) break;
      if (seq > tail + 1) continue;

      if (_tail.compare_exchange(tail, tail + 1, Ordering::Release, Ordering::Acquire)) {
        cell.store(tail + _buf.cap(), Ordering::Release);
        return {offset};
      }
    }
    return {};
  }
};

}  // namespace sfc::sync
