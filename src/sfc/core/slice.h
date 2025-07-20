#pragma once

#include "sfc/core/cmp.h"
#include "sfc/core/iter.h"
#include "sfc/core/ops.h"
#include "sfc/core/option.h"
#include "sfc/core/panicking.h"
#include "sfc/core/tuple.h"

namespace sfc::slice {

template <class T>
struct Iter;

template <class T>
struct Windows;

template <class T>
struct Truncks;

template <class T>
struct Slice {
  T*    _ptr = nullptr;
  usize _len = 0;

 public:
  constexpr Slice() = default;

  constexpr Slice(T* ptr, usize len) : _ptr{ptr}, _len{len} {}

  template <usize N>
  constexpr Slice(T (&v)[N]) : _ptr{v}, _len{N} {}

  auto as_ptr() const -> const T* {
    return _ptr;
  }

  auto as_mut_ptr() -> T* {
    return _ptr;
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
  auto get_unchecked(usize idx) const -> const T& {
    return _ptr[idx];
  }

  auto get_unchecked_mut(usize idx) -> T& {
    return _ptr[idx];
  }

  auto operator[](usize idx) const -> const T& {
    panicking::assert(idx < _len, "Slice::[]: idx(={}) out of range(={})", idx, _len);
    return _ptr[idx];
  }

  auto operator[](usize idx) -> T& {
    panicking::assert(idx < _len, "Slice::[]: idx(={}) out of range(={})", idx, _len);
    return _ptr[idx];
  }

  auto operator[](Range ids) const -> Slice<const T> {
    const auto end = cmp::min(ids._end, _len);
    const auto pos = cmp::min(ids._start, end);
    return {_ptr + pos, end - pos};
  }

  auto operator[](Range ids) -> Slice<T> {
    const auto end = cmp::min(ids._end, _len);
    const auto pos = cmp::min(ids._start, end);
    return {_ptr + pos, end - pos};
  }

  auto split_at(usize mid) const -> tuple::Tuple<Slice<const T>, Slice<const T>> {
    const auto x = cmp::min(mid, _len);
    return tuple::Tuple{{_ptr, x}, {_ptr + x, _len - x}};
  }

  auto split_at_mut(usize mid) -> tuple::Tuple<Slice, Slice> {
    const auto x = cmp::min(mid, _len);
    return tuple::Tuple{{_ptr, x}, {_ptr + x, _len - x}};
  }

 public:
  void swap(usize i, usize j) {
    panicking::assert(i < _len, "Slice::[]: i(={}), out of range(={})", i, _len);
    panicking::assert(j < _len, "Slice::[]: j(={}), out of range(={})", j, _len);

    if (i == j) {
      return;
    }
    mem::swap(_ptr[i], _ptr[j]);
  }

  void fill(T val) {
    for (auto ptr = _ptr, end = ptr + _len; ptr != end; ++ptr) {
      *ptr = val;
    }
  }

  void fill_with(auto&& f) {
    for (auto p = _ptr, e = p + _len; p != e; ++p) {
      *p = f();
    }
  }

  void copy_from_slice(Slice<const T> src) {
    if (_len != src._len) {
      panicking::panic("Slice::copy_from_slice: self.len(={}) != src.len(={})", _len, src._len);
    }

    ptr::copy_nonoverlapping(src._ptr, _ptr, _len);
  }

 public:
  auto operator==(Slice<const T> other) const -> bool {}

  auto contains(const T& x) const -> bool {
    for (auto i = 0U; i < _len; ++i) {
      if (_ptr[i] == x) {
        return true;
      }
    }
    return false;
  }

  auto starts_with(Slice<const T> needle) const -> bool {
    if (_len < needle._len) {
      return false;
    }
    return needle == Slice<const char>{_ptr, needle._len};
  }

  auto ends_with(Slice<const T> needle) const -> bool {
    if (_len < needle._len) {
      return false;
    }
    return needle == Slice<const char>{_ptr + _len - needle._len, needle._len};
  }

 public:
  auto iter() const -> Iter<const T> {
    return Iter<const T>{._ptr = _ptr, ._end = _ptr + _len};
  }

  auto iter_mut() -> Iter<T> {
    return Iter<T>{._ptr = _ptr, ._end = _ptr + _len};
  }

  auto windows(usize n) const -> Windows<const T> {
    return {*this, n};
  }

  auto windows_mut(usize n) -> Windows<T> {
    return {*this, n};
  }

  auto truncks(usize n) const -> Truncks<const T> {
    return {*this, n};
  }

  auto truncks_mut(usize n) -> Truncks<T> {
    return {*this, n};
  }

  void fmt(auto& f) const {
    f.debug_list().entries(this->iter());
  }
};

template <class T>
struct Iter : iter::Iterator<Iter<T>, T&> {
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
};

template <class T>
struct Windows : iter::Iterator<Windows<T>, Slice<T>> {
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
};

template <class T>
struct Truncks : iter::Iterator<Truncks<T>, Slice<T>> {
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
};

template <class T, usize N>
Slice(T (&)[N]) -> Slice<T>;

template <class T>
auto begin(const Slice<T>& self) -> const T* {
  return self._ptr;
}

template <class T>
auto end(const Slice<T>& self) -> const T* {
  return self._ptr + self._len;
}

template <class T>
auto begin(Slice<T>& self) -> T* {
  return self._ptr;
}

template <class T>
auto end(Slice<T>& self) -> T* {
  return self._ptr + self._len;
}

}  // namespace sfc::slice

namespace sfc {
using slice::Slice;
}  // namespace sfc
