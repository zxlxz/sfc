#pragma once

#include "sfc/alloc.h"

namespace sfc::collections {

template <class T>
class [[nodiscard]] VecDeque {
  using Buf = vec::Buf<T>;

  usize _pos{0};
  usize _len{0};
  Buf _buf{};

 public:
  VecDeque() = default;

  ~VecDeque() {
    this->clear();
  }

  VecDeque(VecDeque&& other) noexcept
      : _pos{mem::take(other._pos)}, _len{mem::take(other._len)}, _buf{mem::move(other._buf)} {}

  VecDeque& operator=(VecDeque&& other) noexcept {
    if (this == &other) {
      return *this;
    }
    this->clear();
    _pos = mem::take(other._pos);
    _len = mem::take(other._len);
    _buf = mem::move(other._buf);
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
    return _buf[_buf.wrap_idx(_pos + _len - 1)];
  }

  auto back_mut() -> T& {
    panicking::expect(_len != 0, "VecDeque::back_mut: queue is empty");
    return _buf[_buf.wrap_idx(_pos + _len - 1)];
  }

 public:
  void push_front(T value) {
    if (_len == _buf._cap) {
      this->reserve(1);
    }
    _pos = _buf.wrap_idx(_pos + _buf._cap - 1);
    _len += 1;
    _buf.write(_pos, mem::move(value));
  }

  auto pop_front() -> Option<T> {
    if (_len == 0) {
      return {};
    }

    const auto old_pos = _pos;
    _pos = _buf.wrap_idx(_pos + 1);
    _len -= 1;
    return _buf.read(old_pos);
  }

  void push_back(T value) {
    if (_len == _buf._cap) {
      this->reserve(1);
    }

    const auto pos = _buf.wrap_idx(_pos + _len);
    _len += 1;
    _buf.write(pos, mem::move(value));
  }

  auto pop_back() -> Option<T> {
    if (_len == 0) {
      return {};
    }

    const auto pos = _buf.wrap_idx(_pos + _len - 1);
    _len -= 1;
    return _buf.read(pos);
  }

  void clear() {
    for (auto i = 0UL; i < _len; ++i) {
      mem::drop(_buf[_buf.wrap_idx(_pos + i)]);
    }
    _len = 0;
  }

  void reserve(usize additional) {
    if (_len + additional <= _buf._cap) {
      return;
    }

    const auto req_cap = _len + additional;
    const auto min_cap = _buf._cap < 8U ? 8U : _buf._cap * 2;
    const auto new_cap = req_cap < min_cap ? min_cap : req_cap;

    auto new_buf = Buf::with_capacity(new_cap);
    for (auto i = 0UL; i < _len; ++i) {
      auto tmp = ptr::read(&_buf[_buf.wrap_idx(_pos + i)]);
      new_buf.write(i, mem::move(tmp));
    }
    _pos = 0;
    _buf = mem::move(new_buf);
  }

  void fmt(auto& f) const {
    auto imp = f.debug_list();
    for (auto i = 0UL; i < _len; ++i) {
      auto& val = _buf[_buf.wrap_idx(_pos + i)];
      imp.entry(val);
    }
  }
};

}  // namespace sfc::collections

namespace sfc {
using collections::VecDeque;
}  // namespace sfc
