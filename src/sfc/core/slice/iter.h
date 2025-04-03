#pragma once

#include "mod.h"

namespace sfc::slice {

template <class T>
struct Iter : iter::Iterator<Iter<T>, T&> {
  T* _ptr;
  T* _end;

 public:
  auto len() const -> usize {
    const auto res = _end - _ptr;
    return res >= 0 ? static_cast<usize>(res) : 0U;
  }

  auto next() -> Option<T&> {
    if (_ptr >= _end)
      return {};
    return *(_ptr++);
  }

  auto next_back() -> Option<T&> {
    if (_ptr >= _end)
      return {};
    return *(--_end);
  }
};

template <class T>
struct Windows : iter::Iterator<Windows<T>, T> {
  Slice<T> _buf;
  usize    _len;

 public:
  operator bool() const {
    return _buf._len >= _len;
  }

  auto next() -> Option<Slice<T>> {
    if (_buf._len < _len)
      return {};

    auto res = Slice<T>{_buf._ptr, _len};
    _buf._ptr += 1;
    _buf._len -= 1;
    return res;
  }

  auto next_back() -> Option<Slice<T>> {
    if (_buf._len < _len)
      return {};

    auto res = Slice<T>{_buf._ptr + _buf._len - _len, _len};
    _buf._len -= 1;
    return res;
  }
};

template <class T>
struct Truncks : iter::Iterator<Truncks<T>, T> {
  Slice<T> _buf;
  usize    _len;

 public:
  operator bool() const {
    return _buf._len >= _len;
  }

  auto next() -> Option<Slice<T>> {
    if (_buf._len < _len)
      return {};

    auto res = Slice<T>{_buf._ptr, _len};
    _buf._ptr += _len;
    _buf._len -= _len;
    return res;
  }
};

template <class T>
auto Slice<T>::iter() const -> Iter<const T> {
  return Iter<const T>{{}, _ptr, _ptr + _len};
}

template <class T>
auto Slice<T>::iter_mut() -> Iter<T> {
  return Iter<T>{{}, _ptr, _ptr + _len};
}

template <class T>
auto Slice<T>::windows(usize wlen) const -> Windows<const T> {
  return Windows<const T>{{}, *this, wlen};
}

template <class T>
auto Slice<T>::windows_mut(usize wlen) -> Windows<T> {
  return Windows<T>{{}, *this, wlen};
}

template <class T>
auto Slice<T>::truncks(usize wlen) const -> Truncks<const T> {
  return Truncks<const T>{{}, *this, wlen};
}

template <class T>
auto Slice<T>::truncks_mut(usize wlen) -> Truncks<T> {
  return Truncks<T>{{}, *this, wlen};
}

}  // namespace sfc::slice
