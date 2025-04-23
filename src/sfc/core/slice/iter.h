#pragma once

#include "sfc/core/slice/mod.h"

namespace sfc::slice {

template <class T>
struct Iter {
  using Item = T&;

  T* _ptr;
  T* _end;

 public:
  [[nodiscard]] auto len() const -> usize {
    const auto res = _end - _ptr;
    return res >= 0 ? static_cast<usize>(res) : 0U;
  }

  auto next() -> Option<T&> {
    if (_ptr >= _end) {
      return {};
    }
    _ptr += 1;
    return Option<T&>{*(_ptr - 1)};
  }

  auto next_back() -> Option<T&> {
    if (_ptr >= _end) {
      return {};
    }
    _end -= 1;
    return Option<T&>{*_end};
  }

  auto operator->() -> iter::Iterator<Iter>* {
    return static_cast<iter::Iterator<Iter>*>(this);
  }
};

template <class T>
struct Windows {
  using Item = Slice<T>;

  Slice<T> _buf;
  usize    _len;

 public:
  explicit operator bool() const {
    return _buf._len >= _len;
  }

  auto next() -> Option<Slice<T>> {
    if (_buf._len < _len) {
      return {};
    }

    auto res = Slice<T>{_buf._ptr, _len};
    _buf._ptr += 1;
    _buf._len -= 1;
    return res;
  }

  auto next_back() -> Option<Slice<T>> {
    if (_buf._len < _len) {
      return {};
    }

    auto res = Slice<T>{_buf._ptr + _buf._len - _len, _len};
    _buf._len -= 1;
    return res;
  }

  auto operator->() -> iter::Iterator<Windows>* {
    return static_cast<iter::Iterator<Windows>*>(this);
  }
};

template <class T>
struct Truncks {
  using Item = Slice<T>;

  Slice<T> _buf;
  usize    _len;

 public:
  explicit operator bool() const {
    return _buf._len >= _len;
  }

  auto next() -> Option<Slice<T>> {
    if (_buf._len < _len) {
      return {};
    }

    auto res = Slice<T>{_buf._ptr, _len};
    _buf._ptr += _len;
    _buf._len -= _len;
    return res;
  }

  auto operator->() -> iter::Iterator<Truncks>* {
    return static_cast<iter::Iterator<Truncks>*>(this);
  }
};

template <class T>
auto Slice<T>::iter() const -> Iter<const T> {
  return Iter<const T>{_ptr, _ptr + _len};
}

template <class T>
auto Slice<T>::iter_mut() -> Iter<T> {
  return Iter<T>{_ptr, _ptr + _len};
}

template <class T>
auto Slice<T>::windows(usize wlen) const -> Windows<const T> {
  return Windows<const T>{*this, wlen};
}

template <class T>
auto Slice<T>::windows_mut(usize wlen) -> Windows<T> {
  return Windows<T>{*this, wlen};
}

template <class T>
auto Slice<T>::truncks(usize wlen) const -> Truncks<const T> {
  return Truncks<const T>{*this, wlen};
}

template <class T>
auto Slice<T>::truncks_mut(usize wlen) -> Truncks<T> {
  return Truncks<T>{*this, wlen};
}

}  // namespace sfc::slice
