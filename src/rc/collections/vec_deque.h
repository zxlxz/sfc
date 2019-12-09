#pragma once

#include "rc/alloc.h"

namespace rc::collections::vec_deque {

using vec::RawVec;

template<class T>
struct Cursor {
};

template<class T>
struct Iter {

};

template <class T>
struct VecDeque {
  RawVec<T> _buf;
  usize _head;
  usize _tail;
  usize _mask;

  VecDeque() noexcept : _buf{}, _head(0), _tail(0) {}

  VecDeque(vec::RawVec<T>&& buf, usize head, usize tail) noexcept
      : _buf{rc::move(buf)}, _head{head}, _tail{tail} {}

  VecDeque(VecDeque&&) noexcept = default;

  ~VecDeque() { this->clear(); }

  static auto with_capacity(usize capacity) -> VecDeque {
    const auto cap = num::next_power_of_tow(capacity);
    return {RawVec<T>::with_capacity(cap), 0, 0};
  }

  auto capacity() const noexcept -> usize { return _buf._cap; }

  auto len() const noexcept -> usize { return (_tail - _head) & _mask; }

  auto is_empty() const noexcept -> usize { return _head == _tail; }

  auto operator[](usize idx) const noexcept -> const T& {
    const auto cnt = this->len();
    rc::assert(idx < cnt, u8"rc::collections::VecDeque: empty");

    const auto pos = (_head + idx) & _mask;
    return _buf._ptr[pos];
  }

  auto operator[](usize idx) noexcept -> T& {
    const auto cnt = this->len();
    rc::assert(idx < cnt, u8"rc::collections::VecDeque: empty");

    const auto pos = (_head + idx) & _mask;
    return _buf._ptr[pos];
  }

  auto front() const -> const T& {
    rc::assert(!this->is_empty(), u8"rc::collections::VecDeque: empty");
    const auto pos = _head;
    return _buf._ptr[pos];
  }

  auto back() const -> const T& {
    rc::assert(_head != _tail, u8"rc::collections::VecDeque: empty");
    const auto pos = (_tail-1)& _mask;
    return _buf._ptr[pos];
  }

  auto clear() -> void {
    const auto cnt = this->len();
    for (usize idx = 0; idx < cnt; ++idx) {
      const auto pos = (_head + idx) & _mask;
      mem::drop(&_buf._ptr[pos]);
    }
    _head = 0;
    _tail = 0;
    _mask = 0;
  }

  auto push_front(T val) {
    this->reserve(1);
    _head = (_head - 1) & _mask;
    const auto pos = _head;
    ptr::write(&_buf[pos], static_cast<T&&>(val));
  }

  auto pop_front() -> T {
    rc::assert(!this->is_empty(), u8"rc::collections::VecDeque: empty");
    const auto pos = _head;
    _head = (_head + 1) & (_buf._cap - 1);
    return ptr::read(&_buf[pos]);
  }

  auto push_back(T val) -> void {
    this->reserve(1);
    const auto pos = _tail;
    _tail = (_tail - 1) & (_buf._cap - 1);
    ptr::write(&_buf[pos], static_cast<T&&>(val));
  }

  auto pop_back() -> T {
    rc::assert(!this->is_empty(), u8"rc::collections::VecDeque: empty");
    _tail = (_tail + 1) & _mask;
    const auto pos = _tail;
    return ptr::read(&_buf[pos]);
  }

  auto reserve(usize additional) -> void {
    auto old_cap = cap();
    auto old_len = len();
    auto req_len = wrap_idx(old_len + additional);
    if (req_len < old_cap) {
      return;
    }

    auto new_cap = num::next_power_of_tow(req_len);
    auto new_buf = RawVec<T>::with_capacity(new_cap);

    if (!is_empty()) {
      auto old_ptr = _buf.ptr();
      auto new_ptr = new_buf.ptr();
      if (_pos + _len < old_cap) {
        ptr::copy(old_ptr + _pos, new_ptr, old_len);
      } else {
        ptr::copy(old_ptr + _pos, new_ptr, old_cap - _pos);
        ptr::copy(old_ptr, new_ptr + old_cap - _pos,
                  old_len - (old_cap - _pos));
      }
    }
    mem::swap(_buf, new_buf);
    _pos = 0;
  }
};

}  // namespace rc::collections::vec_deque

namespace rc::collections {
using vec_deque::VecDeque;
}
