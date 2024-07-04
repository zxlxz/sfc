#pragma once

#include "sfc/alloc.h"

namespace sfc::collections {

template <class T>
class CircBuf {
  vec::Buf<T> _buf;
  usize _len = 0;
  usize _first = 0;  // begin of the buffer
  usize _last = 0;   // end of the buffer(behind the last element)

 public:
  CircBuf() = default;

  explicit CircBuf(usize capacity) : _buf{_buf.with_capacity(capacity)} {}

  ~CircBuf() {
    this->clear();
  }

  static auto with_capacity(usize capacity) -> CircBuf {
    return CircBuf{capacity};
  }

  auto len() const -> usize {
    return _len;
  }

  auto capacity() const -> usize {
    return _buf.capacity();
  }

  auto is_empty() const -> bool {
    return _len == 0;
  }

  auto is_full() const -> bool {
    return _len == _buf.capacity();
  }

  void clear() {
    for (; _len != 0;) {
      mem::drop(_buf[_first]);
      this->wrap_inc(_first);
      _len -= 1;
    }
  }

  [[sfc_inline]] auto operator[](usize idx) const -> const T& {
    assert_fmt(idx < _len, "CircBuf::[]: idx(={}) out of range(={})", idx, _len);

    const auto offset = this->warp_idx(idx);
    return _buf[offset];
  }

  [[sfc_inline]] auto operator[](usize idx) -> T& {
    assert_fmt(idx < _len, "CircBuf::[]: idx(={}) out of range(={})", idx, _len);

    const auto offset = this->warp_idx(idx);
    return _buf[offset];
  }

  [[sfc_inline]] auto front() const -> const T& {
    assert_fmt(!this->is_empty(), "CircBuf::front: buf is empty");
    return _buf[_first];
  }

  [[sfc_inline]] auto back() const -> const T& {
    assert_fmt(!this->is_empty(), "CircBuf::front: buf is empty");
    const auto idx = (_last == 0 ? _buf.capacity() : _last) - 1;
    return _buf[idx];
  }

  void push_front(T val) {
    if (this->is_full()) {
      if (this->is_empty()) {
        return;
      }
      this->wrap_dec(_first);
      mem::replace(_buf[_first], val);
      _last = _first;
    } else {
      this->wrap_dec(_first);
      ptr::write(&_buf[_first], static_cast<T&&>(val));
      _len += 1;
    }
  }

  void push_back(T val) {
    if (this->is_full()) {
      if (this->is_empty()) {
        return;
      }
      _buf[_last] = static_cast<T&&>(val);
      this->wrap_inc(_last);
      _first = _last;
    } else {
      ptr::write(&_buf[_last], static_cast<T&&>(val));
      this->wrap_inc(_last);
      _len += 1;
    }
  }

  auto pop_front() -> Option<T> {
    auto ret = static_cast<T&&>(_buf[_first]);
    mem::drop(_buf[_first]);
    this->wrap_inc(_first);

    _len -= 1;
    return ret;
  }

  auto pop_back() -> Option<T> {
    this->wrap_dec(_last);
    auto ret = static_cast<T&&>(_buf[_last]);
    mem::drop(_buf[_last]);

    _len -= 1;
    return ret;
  }

 private:
  [[sfc_inline]] auto warp_idx(usize idx) const -> usize {
    const auto cap = _buf.capacity();

    auto pos = _first + idx;
    if (pos >= cap) {
      pos -= cap;
    }
    return pos;
  }

  [[sfc_inline]] void wrap_inc(usize& pos) {
    pos += 1;
    if (pos == _buf.capacity()) {
      pos = 0;
    }
  }

  [[sfc_inline]] void wrap_dec(usize& pos) {
    if (pos == 0) {
      pos = _buf.capacity();
    }
    pos -= 1;
  }
};

}  // namespace sfc::collections

namespace sfc {
using collections::CircBuf;
}  // namespace sfc
