#pragma once

#include "sfc/alloc.h"

namespace sfc::collections {

template <class T>
class [[nodiscard]] VecDeque {
  usize _pos{0};
  usize _len{0};
  vec::Buf<T> _buf{};

 public:
  VecDeque() = default;

  ~VecDeque() {
    this->clear();
  }

  VecDeque(VecDeque&& other) noexcept
      : _pos{mem::take(other._pos)}, _len{mem::take(other._len)}, _buf{mem::move(other._buf)} {}

  VecDeque& operator=(VecDeque&& other) noexcept {
    if (this != &other) {
      this->clear();
      _pos = mem::take(other._pos);
      _len = mem::take(other._len);
      _buf = mem::move(other._buf);
    }
    return *this;
  }

  static auto with_capacity(usize capacity) -> VecDeque {
    auto res = VecDeque{};
    res._buf.reserve(0, capacity);
    return res;
  }

  auto capacity() const -> usize {
    return _buf._cap;
  }

  auto len() const -> usize {
    return _len;
  }

  auto is_empty() const -> bool {
    return _len == 0;
  }

  explicit operator bool() const {
    return _len != 0;
  }

 public:
  auto front() const -> const T& {
    panicking::expect(_len != 0, "VecDeque::front: queue is empty");
    return _buf[_pos];
  }

  auto front_mut() -> T& {
    panicking::expect(_len != 0, "VecDeque::front_mut: queue is empty");
    return _buf[_pos];
  }

  auto back() const -> const T& {
    panicking::expect(_len != 0, "VecDeque::back: queue is empty");
    return _buf[this->wrap_idx(_pos + _len - 1)];
  }

  auto back_mut() -> T& {
    panicking::expect(_len != 0, "VecDeque::back_mut: queue is empty");
    return _buf[this->wrap_idx(_pos + _len - 1)];
  }

 public:
  void push_front(T value) {
    if (_len == _buf._cap) {
      this->reserve(1);
    }
    _pos = this->wrap_idx(_pos + _buf._cap - 1);
    _len += 1;
    ptr::write(&_buf[_pos], mem::move(value));
  }

  auto pop_front() -> Option<T> {
    if (_len == 0) {
      return {};
    }

    auto res = ptr::read(&_buf[_pos]);
    _pos = this->wrap_idx(_pos + 1);
    _len -= 1;

    return res;
  }

  void push_back(T value) {
    if (_len == _buf._cap) {
      this->reserve(1);
    }

    const auto pos = this->wrap_idx(_pos + _len);
    ptr::write(&_buf[pos], mem::move(value));
    _len += 1;
  }

  auto pop_back() -> Option<T> {
    if (_len == 0) {
      return {};
    }

    const auto pos = this->wrap_idx(_pos + _len - 1);
    auto res = ptr::read(&_buf[pos]);
    _len -= 1;
    return res;
  }

  void clear() {
    for (auto i = 0UL; i < _len; ++i) {
      mem::drop(_buf[this->wrap_idx(_pos + i)]);
    }
    _len = 0;
  }

  void reserve(usize additional) {
    if (_len + additional <= _buf._cap) {
      return;
    }

    const auto min_cap = num::max(_buf._cap * 2, usize{8U});
    const auto new_cap = num::min(_len + additional, min_cap);
    auto new_buf = _buf.with_capacity(new_cap);

    for (auto i = 0UL; i < _len; ++i) {
      auto tmp = ptr::read(&_buf[this->wrap_idx(_pos + i)]);
      ptr::write(&new_buf[i], mem::move(tmp));
    }
    _pos = 0;
    _buf = mem::move(new_buf);
  }

 public:
  // trait: fmt::Display
  void fmt(auto& f) const {
    auto imp = f.debug_list();
    for (auto i = 0UL; i < _len; ++i) {
      auto& val = _buf[_buf.wrap_idx(_pos + i)];
      imp.entry(val);
    }
  }

 private:
  auto wrap_idx(usize idx) const -> usize {
    return idx < _buf._cap ? idx : idx - _buf._cap;
  }
};

}  // namespace sfc::collections

namespace sfc {
using collections::VecDeque;
}  // namespace sfc
