#pragma once

#include "sfc/core/cmp.h"
#include "sfc/core/iter.h"
#include "sfc/core/ops.h"
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
  constexpr explicit Slice(T (&v)[N]) : _ptr{v}, _len{N} {}

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

  auto get_unchecked(Range ids) const -> Slice<const T> {
    return {_ptr + ids._start, ids.len()};
  }

  auto get_unchecked_mut(Range ids) -> Slice<T> {
    return {_ptr + ids._start, ids.len()};
  }

  auto operator[](usize idx) const -> const T& {
    assert_fmt(idx < _len, "Slice::[]: idx(={}) out of range(={})", idx, _len);
    return _ptr[idx];
  }

  auto operator[](usize idx) -> T& {
    assert_fmt(idx < _len, "Slice::[]: idx(={}) out of range(={})", idx, _len);
    return _ptr[idx];
  }

  auto operator[](Range ids) const -> Slice<const T> {
    ids = ids % _len;
    return {_ptr + ids._start, ids.len()};
  }

  auto operator[](Range ids) -> Slice<T> {
    ids = ids % _len;
    return {_ptr + ids._start, ids.len()};
  }

  auto split_at(usize idx) const -> tuple::Tuple<Slice<const T>, Slice<const T>> {
    idx = cmp::min(idx, _len);
    return {this->get_unchecked({0, idx}), this->get_unchecked({idx, _len})};
  }

  auto split_at_mut(usize idx) -> tuple::Tuple<Slice, Slice> {
    idx = cmp::min(idx, _len);
    return {this->get_unchecked_mut({0, idx}), this->get_unchecked_mut({idx, _len})};
  }

 public:
  void swap(usize i, usize j) {
    assert_fmt(i < _len, "Slice::[]: i(={}), out of range(={})", i, _len);
    assert_fmt(j < _len, "Slice::[]: j(={}), out of range(={})", j, _len);

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
      panicking::panic_fmt("Slice::copy_from_slice: self.len(={}) != src.len(={})", _len, src._len);
    }

    ptr::copy_nonoverlapping(src._ptr, _ptr, _len);
  }

 public:
  auto iter() const -> Iter<const T>;
  auto iter_mut() -> Iter<T>;

  auto windows(usize n) const -> Windows<const T>;
  auto windows_mut(usize n) -> Windows<T>;

  auto truncks(usize n) const -> Truncks<const T>;
  auto truncks_mut(usize n) -> Truncks<T>;

  void fmt(auto& f) const {
    f.debug_list().entries(this->iter());
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
