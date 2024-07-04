#pragma once

#include "mod.h"

namespace sfc::slice {

template <class T>
struct Iter : iter::Iterator<Iter<T>, T&> {
  T* _ptr;
  T* _end;

 public:
  [[sfc_inline]] Iter(T* ptr, usize len) noexcept : _ptr{ptr}, _end{_ptr + len} {}

  [[sfc_inline]] auto len() const -> usize {
    const auto res = _end - _ptr;
    return res >= 0 ? static_cast<usize>(res) : 0U;
  }

  [[sfc_inline]] auto next() -> Option<T&> {
    if (_ptr >= _end) return {};
    return *(_ptr++);
  }

  [[sfc_inline]] auto next_back() -> Option<T&> {
    if (_ptr >= _end) return {};
    return *(--_end);
  }
};

template <class T>
auto Slice<T>::iter() const -> Iter {
  return {_ptr, _len};
}

template <class T>
auto Slice<T>::iter_mut() -> IterMut {
  return {_ptr, _len};
}

template <class T>
struct Windows : iter::Iterator<Windows<T>, T> {
  Slice<T> _buf;
  usize _len;

 public:
  Windows(Slice<T> buf, usize wlen) noexcept : _buf{buf}, _len{wlen} {}

  [[sfc_inline]] operator bool() const {
    return _buf._len >= _len;
  }

  [[sfc_inline]] auto next() -> Option<Slice<T>> {
    if (_buf._len < _len) return {};

    auto res = Slice<T>{_buf._ptr, _len};
    _buf._ptr += 1;
    _buf._len -= 1;
    return res;
  }

  [[sfc_inline]] auto next_back() -> Option<Slice<T>> {
    if (_buf._len < _len) return {};

    auto res = Slice<T>{_buf._ptr + _buf._len - _len, _len};
    _buf._len -= 1;
    return res;
  }
};

template <class T>
auto Slice<T>::windows(usize wlen) const {
  return Windows<const T>{*this, wlen};
}

template <class T>
auto Slice<T>::windows_mut(usize wlen) {
  return Windows<T>{*this, wlen};
}

template <class T>
struct Truncks : iter::Iterator<Truncks<T>, T> {
  Slice<T> _buf;
  usize _len;

 public:
  Truncks(Slice<T> buf, usize wlen) noexcept : _buf{buf}, _len{wlen} {}

  [[sfc_inline]] operator bool() const {
    return _buf._len >= _len;
  }

  [[sfc_inline]] auto next() -> Option<Slice<T>> {
    if (_buf._len < _len) return {};

    auto res = Slice<T>{_buf._ptr, _len};
    _buf._ptr += _len;
    _buf._len -= _len;
    return res;
  }
};

template <class T>
auto Slice<T>::truncks(usize wlen) const {
  return Truncks<const T>{*this, wlen};
}

template <class T>
auto Slice<T>::truncks_mut(usize wlen) {
  return Truncks<T>{*this, wlen};
}

}  // namespace sfc::slice
