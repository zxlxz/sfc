#pragma once

#include "sfc/alloc/vec.h"

namespace sfc::collections {

template <class T, class A = alloc::Global>
class [[nodiscard]] Queue {
  using Buf = RawVec<T, A>;

  Buf _buf{};
  usize _len{0};
  usize _head{0};

 public:
  Queue() = default;

  ~Queue() noexcept {
    this->clear();
  }

  Queue(Queue&& other) noexcept
      : _buf{mem::move(other._buf)}, _len{mem::take(other._len)}, _head{mem::take(other._head)} {}

  Queue& operator=(Queue&& other) noexcept {
    if (this != &other) {
      mem::swap(_buf, other._buf);
      mem::swap(_len, other._len);
      mem::swap(_head, other._head);
    }
    return *this;
  }

  static auto with_capacity(usize capacity) -> Queue {
    auto res = Queue{};
    res.reserve(capacity);
    return res;
  }

  auto capacity() const noexcept -> usize {
    return _buf._cap;
  }

  auto len() const noexcept -> usize {
    return _len;
  }

  auto is_empty() const noexcept -> bool {
    return _len == 0;
  }

  auto is_contiguous() const noexcept -> bool {
    return _len == 0 || _head + _len <= _buf._cap;
  }

 public:
  auto top() const noexcept -> Option<const T&> {
    if (_len == 0) {
      return {};
    }
    return Option<const T&>{_buf._ptr[_head]};
  }

  auto top_mut() noexcept -> Option<T&> {
    if (_len == 0) {
      return {};
    }
    return Option<T&>{_buf._ptr[_head]};
  }

  void push(T value) noexcept {
    if (_len == _buf._cap) {
      this->reserve(1);
    }

    const auto new_tail = this->to_physical_index(_len);
    ptr::write(_buf._ptr + new_tail, static_cast<T&&>(value));
    _len += 1;
  }

  auto pop() noexcept -> Option<T> {
    if (_len == 0) {
      return {};
    }

    const auto old_head = _head;
    _head = this->to_physical_index(1);
    _len -= 1;
    return Option<T>{ptr::read(_buf._ptr + old_head)};
  }

 public:
  void clear() noexcept {
    if (_len == 0) {
      return;
    }
    this->for_each([&](T& x) { x.~T(); });
    _len = 0;
    _head = 0;
  }

  void reserve(usize additional) noexcept {
    if (_len + additional <= _buf._cap) {
      return;
    }
    const auto new_cap = num::next_power_of_two(_len + additional);
    this->reserve_exact(new_cap - _len);
  }

  void reserve_exact(usize additional) noexcept {
    if (_len + additional <= _buf._cap) {
      return;
    }

    const auto new_cap = _len + additional;
    if (this->is_contiguous()) {
      _buf.realloc(_len, new_cap);
    } else {
      auto new_buf = Buf::with_capacity(new_cap);
      ptr::uninit_move(_buf._ptr + _head, new_buf._ptr, _buf._cap - _head);
      ptr::uninit_move(_buf._ptr, new_buf._ptr + (_buf._cap - _head), _head);
      _buf = mem::move(new_buf);
      _head = 0;
    }
  }

 public:
  auto to_physical_index(usize idx) const noexcept -> usize {
    const auto logic_idx = _head + idx;
    return logic_idx < _buf._cap ? logic_idx : logic_idx - _buf._cap;
  }

  void for_each(this auto&& self, auto&& f) {
    for (auto i = 0UL; i < self._len; ++i) {
      const auto off = self.to_physical_index(i);
      f(self._buf._ptr[off]);
    }
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    auto imp = f.debug_list();
    this->for_each([&](const T& val) { imp.entry(val); });
  }
};

}  // namespace sfc::collections
