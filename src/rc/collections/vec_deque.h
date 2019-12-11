#pragma once

#include "rc/alloc.h"

namespace rc::collections::vec_deque {

using vec::RawVec;

template <class T>
struct Cursor {
  T* _ptr;
  usize _mask;
  usize _idx;

  auto operator++() noexcept -> void { _idx = (_idx + 1) & _mask; }
  auto operator--() noexcept -> void { _idx = (_idx - 1) & _mask; }
  auto operator*() const noexcept -> T& { return _ptr[_idx]; }

  auto operator!=(const Cursor& other) const noexcept -> bool {
    return _idx != other._idx;
  }
};

template <class T>
struct Iter: iter::Iterator<Iter<T>> {
  using Item = T&;

  T* _ptr;
  usize _mask;
  usize _head;
  usize _tail;

  auto next() noexcept -> Option<T&> {
    if (_head == _tail) return {};
    auto& res = _ptr[_head];
    _head = (_head + 1) & _mask;
    return {res};
  }

  auto next_back() noexcept -> Option<T&> {
    if (_head == _tail) return {};
    _tail = (_tail - 1) & _mask;
    return {_ptr[_tail]};
  }
};

template <class T>
struct VecDeque {

  using Iter = vec_deque::Iter<const T>;
  using IterMut = vec_deque::Iter<T>;
  using Cursor = vec_deque::Cursor<const T>;
  using CursorMut = vec_deque::Cursor<T>;

  RawVec<T> _buf;
  usize _mask;
  usize _head;
  usize _tail;

  VecDeque() noexcept : _buf{}, _head(0), _tail(0), _mask{0} {}

  VecDeque(vec::RawVec<T>&& buf, usize head, usize tail) noexcept
      : _buf{rc::move(buf)}, _head{head}, _tail{tail}, _mask{0} {}

  VecDeque(VecDeque&& other) noexcept
      : _buf{rc::move(other._buf)},
        _head{other._head}, _tail{other._tail} {
    ptr::write(&other, VecDeque{});
  }

  ~VecDeque() { this->clear(); }

  static auto with_capacity(usize capacity) -> VecDeque {
    const auto cap = num::next_power_of_tow(capacity);
    return {RawVec<T>::with_capacity(cap), 0, 0};
  }

  auto capacity() const noexcept -> usize {
    return _buf._cap == 0 ? 0 : _buf._cap - 1;
  }

  auto len() const noexcept -> usize { return (_tail - _head) & _mask; }

  auto is_empty() const noexcept -> usize { return _head == _tail; }

  auto begin() const noexcept -> Cursor {
    return Cursor{_buf._ptr, _buf._cap-1, _head};
  }

  auto end() const noexcept -> Cursor {
    return Cursor{_buf._ptr, _buf._cap-1, _tail};
  }

  auto begin() noexcept -> CursorMut {
    return CursorMut{_buf._ptr, _buf._cap - 1, _head};
  }

  auto end() noexcept -> CursorMut {
    return CursorMut{_buf._ptr, _buf._cap - 1, _tail};
  }

  auto _wrap_idx(usize idx) const -> usize { return idx & (_buf._cap - 1); }

  constexpr auto get_unchecked(usize idx) const noexcept -> const T& {
    return _buf._ptr[_wrap_idx(_head + idx)];
  }

  constexpr auto get_unchecked_mut(usize index) noexcept -> T& {
    return _buf._ptr[_wrap_idx(_head + idx)];
  }

  auto operator[](usize idx) const noexcept -> const T& {
    const auto cnt = this->len();
    rc::assert(idx < cnt, u8"rc::collections::VecDeque: empty");
    return this->get_unchecked(idx);
  }

  auto operator[](usize idx) noexcept -> T& {
    const auto cnt = this->len();
    rc::assert(idx < cnt, u8"rc::collections::VecDeque: empty");
    return this->get_unchecked_mut(idx);
  }

  auto front() const -> const T& {
    rc::assert(!this->is_empty(), u8"rc::collections::VecDeque: empty");
    return _buf._ptr[_head];
  }

  auto back() const -> const T& {
    rc::assert(_head != _tail, u8"rc::collections::VecDeque: empty");
    return _buf._ptr[_wrap_idx(_tail - 1)];
  }

  auto clear() -> void {
    const auto cnt = this->len();
    for (auto& val: *this) {
      mem::drop(val);
    }
    _head = 0;
    _tail = 0;
    _mask = 0;
  }

  auto push_front(T val) {
    this->reserve(1);
    _head = _wrap_idx(_head - 1);
    ptr::write(&_buf._ptr[_head], static_cast<T&&>(val));
  }

  auto pop_front() -> T {
    rc::assert(!this->is_empty(), u8"rc::collections::VecDeque: empty");
    const auto old_head = _head;
    _head = _wrap_idx(_head + 1);
    return ptr::read(&_buf._ptr[old_head]);
  }

  auto push_back(T val) -> void {
    this->reserve(1);
    const auto old_tail = _tail;
    _tail = _wrap_idx(_tail + 1);
    ptr::write(&_buf._ptr[old_tail], static_cast<T&&>(val));
  }

  auto pop_back() -> T {
    rc::assert(!this->is_empty(), u8"rc::collections::VecDeque: empty");
    _tail = _wrap_idx(_tail - 1);
    return ptr::read(&_buf[_tail]);
  }

  auto reserve(usize additional) -> void {
    const auto old_cap = this->capacity();
    const auto old_len = this->len();
    const auto req_len = old_len + additional;
    if (req_len < old_cap) return;

    const auto new_cap = num::next_power_of_two(req_len);
    auto new_buf = RawVec<T>::with_capacity(new_cap);

    if (!this->is_empty()) {
      const auto old_ptr = _buf._ptr;
      const auto new_ptr = new_buf._ptr;
      if (_head + old_len < _tail) {
        ptr::copy(old_ptr + _head, new_ptr, old_len);
      } else {
        const auto len1 = old_cap - _head;
        const auto len2 = old_len - len1;
        ptr::copy(old_ptr + _head, new_ptr, len1);
        ptr::copy(old_ptr, new_ptr + len1, len2);
      }
    }
    mem::swap(_buf, new_buf);
    _head = 0;
    _tail = old_len;
    _mask = new_cap - 1;
  }

  template<class Out>
  void fmt(fmt::Formatter<Out>& formatter) const {
    auto dbg = formatter.debug_list();
    for(const auto& t: *this) {
      dbg.entry(t);
    }
  }
};

}  // namespace rc::collections::vec_deque

namespace rc::collections {
using vec_deque::VecDeque;
}
