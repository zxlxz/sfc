#pragma once

#include "sfc/alloc/vec.h"

namespace sfc::collections::queue {

template <class T>
struct Iter : iter::Iterator<T&> {
  slice::Iter<T> _iter1;
  slice::Iter<T> _iter2;

 public:
  auto len() const noexcept -> usize {
    return _iter1.len() + _iter2.len();
  }

  auto next() noexcept -> Option<T&> {
    if (auto t = _iter1.next()) return t;
    return _iter2.next();
  }

  auto next_back() noexcept -> Option<T> {
    if (auto t = _iter1.next_back()) return t;
    return _iter2.next_back();
  }
};

template <class T, class A = alloc::Global>
class [[nodiscard]] Queue {
  usize _head{0};
  usize _len{0};
  RawVec<T, A> _buf{};

 public:
  Queue() noexcept = default;

  ~Queue() noexcept {
    this->clear();
  }

  Queue(Queue&& other) noexcept
      : _head{mem::take(other._head)}, _len{mem::take(other._len)}, _buf{mem::move(other._buf)} {}

  Queue& operator=(Queue&& other) noexcept {
    if (this != &other) {
      mem::swap(_head, other._head);
      mem::swap(_len, other._len);
      mem::swap(_buf, other._buf);
    }
    return *this;
  }

  static auto with_capacity(usize capacity) -> Queue {
    auto res = Queue{};
    res.reserve(capacity);
    return res;
  }

  auto capacity() const noexcept -> usize {
    return _buf.cap();
  }

  auto len() const noexcept -> usize {
    return _len;
  }

  auto is_empty() const noexcept -> bool {
    return _len == 0;
  }

  auto is_full() const noexcept -> bool {
    return _len == _buf.cap();
  }

  auto is_contiguous() const noexcept -> bool {
    return _head + _len <= _buf.cap();
  }

 public:
  auto top() const noexcept -> Option<const T&> {
    if (_len == 0) {
      return {};
    }
    return _buf[_head];
  }

  auto top_mut() noexcept -> Option<T&> {
    if (_len == 0) {
      return {};
    }
    return _buf[_head];
  }

  void push(T value) noexcept {
    if (_len == _buf.cap()) {
      this->reserve(1);
    }

    const auto new_tail = this->to_physical_idx(_len);
    ptr::write(&_buf[new_tail], static_cast<T&&>(value));
    _len += 1;
  }

  auto pop() noexcept -> Option<T> {
    if (_len == 0) {
      return {};
    }

    const auto old_head = _head;
    _head = this->to_physical_idx(1);
    _len -= 1;
    return ptr::read(&_buf[old_head]);
  }

  void clear() noexcept {
    if (_len == 0) {
      return;
    }
    auto [s1, s2] = this->as_mut_slices();
    ptr::drop(s1._ptr, s1._len);
    ptr::drop(s2._ptr, s2._len);
    _len = 0;
    _head = 0;
  }

  void reserve(usize additional) noexcept {
    const auto new_cap = _len + additional;
    const auto old_cap = _buf.cap();
    if (new_cap <= old_cap) {
      return;
    }

    _buf.reserve(_len, additional);
    this->handle_capacity_increase(old_cap);
  }

  void reserve_exact(usize additional) noexcept {
    const auto new_cap = _len + additional;
    const auto old_cap = _buf.cap();
    if (new_cap <= old_cap) {
      return;
    }

    _buf.reserve_exact(_len, additional);
    this->handle_capacity_increase(old_cap);
  }

 public:
  auto as_slices() const -> Tuple<Slice<const T>, Slice<const T>> {
    const auto p = const_cast<const T*>(_buf.ptr());
    if (_head + _len <= _buf.cap()) {
      return {Slice{p + _head, _len}, {}};
    } else {
      const auto n = _buf.cap() - _head;
      return {Slice{p + _head, n}, Slice{p, _len - n}};
    }
  }

  auto as_mut_slices() -> Tuple<Slice<T>, Slice<T>> {
    const auto p = _buf.ptr();
    if (_head + _len <= _buf.cap()) {
      return {Slice{p + _head, _len}, {}};
    } else {
      const auto n = _buf.cap() - _head;
      return {Slice{p + _head, n}, Slice{p, _len - n}};
    }
  }

  using Iter = queue::Iter<const T>;
  auto iter() const -> Iter {
    const auto [s1, s2] = this->as_slices();
    return Iter{{}, s1.iter(), s2.iter()};
  }

  using IterMut = queue::Iter<T>;
  auto iter_mut() -> IterMut {
    const auto [s1, s2] = this->as_mut_slices();
    return IterMut{{}, s1.iter_mut(), s2.iter_mut()};
  }

 public:
  // trait: fmt::Display
  void fmt(auto& f) const {
    f.debug_list().entries(this->iter());
  }

 private:
  auto to_physical_idx(usize idx) const noexcept -> usize {
    const auto cap = _buf.cap();
    const auto pos = idx + _head;
    return pos < cap ? pos : pos - cap;
  }

  void handle_capacity_increase(usize old_cap) noexcept {
    if (_len == 0) {
      return;
    }

    if (_head <= old_cap - _len) {
      return;
    }

    const auto ptr = _buf.ptr();
    const auto new_cap = _buf.cap();
    const auto head_len = old_cap - _head;
    const auto tail_len = _len - head_len;
    if (head_len > tail_len && old_cap + tail_len <= new_cap) {
      ptr::copy_nonoverlapping(ptr, ptr + old_cap, tail_len);
    } else {
      const auto new_head = new_cap - head_len;
      ptr::copy(ptr + _head, ptr + new_head, head_len);
      _head = new_head;
    }
  }
};

}  // namespace sfc::collections::queue

namespace sfc::collections {
using queue::Queue;
}
