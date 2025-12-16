#pragma once

#include "sfc/alloc/vec.h"

namespace sfc::collections::vec_deque {

template <class T, class A = alloc::Global>
class [[nodiscard]] VecDeque {
  using Buf = vec::Buf<T, A>;

  Buf _buf{};
  usize _len{0};
  usize _pos{0};

 public:
  VecDeque() = default;

  ~VecDeque() noexcept {
    this->clear();
  }

  VecDeque(VecDeque&& other) noexcept
      : _buf{mem::move(other._buf)}, _len{mem::take(other._len)}, _pos{mem::take(other._pos)} {}

  VecDeque& operator=(VecDeque&& other) noexcept {
    if (this != &other) {
      mem::swap(_buf, other._buf);
      mem::swap(_len, other._len);
      mem::swap(_pos, other._pos);
    }
    return *this;
  }

  static auto with_capacity(usize capacity) -> VecDeque {
    auto res = VecDeque{};
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

 public:
  auto front() const noexcept -> const T& {
    panicking::expect(_len != 0, "VecDeque::front: queue is empty");
    return _buf._ptr[_pos];
  }

  auto front_mut() noexcept -> T& {
    panicking::expect(_len != 0, "VecDeque::front_mut: queue is empty");
    return _buf._ptr[_pos];
  }

  auto back() const noexcept -> const T& {
    panicking::expect(_len != 0, "VecDeque::back: queue is empty");
    const auto off = (_pos + _len - 1) & (_buf._cap - 1);
    return _buf._ptr[off];
  }

  auto back_mut() noexcept -> T& {
    panicking::expect(_len != 0, "VecDeque::back_mut: queue is empty");
    const auto off = (_pos + _len - 1) & (_buf._cap - 1);
    return _buf._ptr[off];
  }

 public:
  void push_front(T value) noexcept {
    if (_len == _buf._cap) {
      this->reserve(1);
    }
    const auto off = (_pos - 1) & (_buf._cap - 1);
    ptr::write(_buf._ptr + off, static_cast<T&&>(value));
    _pos = off;
    _len += 1;
  }

  auto pop_front() noexcept -> Option<T> {
    if (_len == 0) {
      return {};
    }

    auto res = Option{ptr::read(_buf._ptr + _pos)};
    _pos = (_pos + 1) & (_buf._cap - 1);
    _len -= 1;
    return res;
  }

  void push_back(T value) noexcept {
    if (_len == _buf._cap) {
      this->reserve(1);
    }

    const auto off = (_pos + _len) & (_buf._cap - 1);
    ptr::write(_buf._ptr + off, static_cast<T&&>(value));
    _len += 1;
  }

  auto pop_back() noexcept -> Option<T> {
    if (_len == 0) {
      return {};
    }

    const auto off = (_pos + _len - 1) & (_buf._cap - 1);
    auto res = Option{ptr::read(_buf._ptr + off)};
    _len -= 1;
    return res;
  }

  void clear() noexcept {
    if (_len == 0) {
      return;
    }

    for (auto i = 0UL; i < _len; ++i) {
      const auto off = (_pos + i) & (_buf._cap - 1);
      mem::drop(_buf._ptr[off]);
    }
    _len = 0;
    _pos = 0;
  }

  void reserve(usize additional) noexcept {
    if (_len + additional <= _buf._cap) {
      return;
    }

    const auto new_cap = num::next_power_of_two(_len + additional);
    auto tmp = Buf::with_capacity(new_cap);

    for (auto i = 0UL; i < _len; ++i) {
      const auto off = (_pos + i) & (_buf._cap - 1);
      auto& src = _buf._ptr[off];
      ptr::write(tmp._ptr + i, static_cast<T&&>(src));
      src.~T();
    }
    _pos = 0;
    _buf = mem::move(tmp);
  }

 public:
  void for_each(this auto&& self, auto&& f) {
    const auto ptr = self._buf._ptr;
    const auto msk = self._buf._cap - 1;
    for (auto i = 0UL; i < self._len; ++i) {
      const auto off = (self._pos + i) & msk;
      f(ptr[off]);
    }
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    auto imp = f.debug_list();
    this->for_each([&](const T& val) { imp.entry(val); });
    imp.finish();
  }
};

}  // namespace sfc::collections::vec_deque

namespace sfc::collections {
using vec_deque::VecDeque;
}
