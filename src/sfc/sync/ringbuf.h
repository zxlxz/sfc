#pragma once

#include "sfc/alloc/alloc.h"

namespace sfc::sync::ringbuf {

struct AtomicIdx {
  Atomic<usize> _val{0};

 public:
  auto value() const noexcept -> usize {
    return _val.load(Ordering::Acquire);
  }

  auto test_inc(usize val) noexcept -> bool {
    return _val.compare_exchange(val, val + 1, Ordering::AcqRel, Ordering::Acquire);
  }
};

template <class T>
struct Cell {
  Atomic<usize> _seq{0};
  T _val{};

 public:
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

template <class T>
class RingBuf {
  using Idx = AtomicIdx;
  using Buf = alloc::RawBuf<Cell<T>>;
  Idx _head{};
  Idx _tail{};
  Buf _buf{};

 public:
  RingBuf() noexcept {}

  ~RingBuf() noexcept {
    this->clear();
  }

  RingBuf(RingBuf&& other) noexcept = default;
  RingBuf& operator=(RingBuf&& other) noexcept = default;

  static auto with_capacity(usize capacity) -> RingBuf {
    auto res = RingBuf{};
    res._buf = Buf::with_capacity(capacity);

    // init
    for (auto i = 0U; i < capacity; ++i) {
      res._buf[i].init(i);
    }
    return res;
  }

 public:
  auto capacity() const noexcept -> usize {
    return _buf.cap();
  }

  auto len() const noexcept -> usize {
    return _head.value() - _tail.value();
  }

  auto is_empty() const noexcept -> bool {
    return this->len() == 0;
  }

  auto is_full() const noexcept -> bool {
    return this->len() == _buf.cap();
  }

 public:
  auto push(T val) noexcept -> Option<T> {
    if (this->try_push(val)) {
      return {};
    }
    return Option<T>{mem::move(val)};
  }

  auto try_push(T& val) -> bool {
    if (_buf.cap() == 0) {
      return false;
    }

    while (true) {
      const auto head = _head.value();
      auto& cell = _buf[head % _buf.cap()];

      if (auto seq = cell.seq(); seq != head) {
        if (seq < head) break;
        if (seq > head) continue;
      }

      if (_head.test_inc(head)) {
        const auto new_seq = head + 1;
        cell.set(new_seq, mem::move(val));
        return true;
      }
    }
    return false;
  }

  auto pop() noexcept -> Option<T> {
    if (_buf.cap() == 0) {
      return Option<T>{};
    }

    while (true) {
      const auto tail = _tail.value();
      auto& cell = _buf[tail % _buf.cap()];

      if (auto seq = cell.seq(); seq != tail + 1) {
        if (seq < tail + 1) break;
        if (seq > tail + 1) continue;
      }

      if (_tail.test_inc(tail)) {
        const auto new_seq = tail + _buf.cap();
        return Option<T>{cell.take(new_seq)};
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

}  // namespace sfc::sync::ringbuf

namespace sfc::sync {
using ringbuf::RingBuf;
}
