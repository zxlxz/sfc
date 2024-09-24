#pragma once

#include "sfc/core/cmp.h"
#include "sfc/core/iter.h"
#include "sfc/core/ops.h"
#include "sfc/core/tuple.h"

namespace sfc::slice {

template <class T>
struct Iter;

template <class T>
struct Slice {
  T* _ptr;
  usize _len;

 public:
  [[sfc_inline]] Slice() : _ptr{nullptr}, _len{0} {}

  [[sfc_inline]] Slice(T* ptr, usize len) : _ptr{ptr}, _len{len} {}

  template <usize N>
  [[sfc_inline]] Slice(T (&v)[N]) : _ptr{v}, _len{N} {}

  [[sfc_inline]] auto as_ptr() const -> const T* {
    return _ptr;
  }

  [[sfc_inline]] auto as_mut_ptr() -> T* {
    return _ptr;
  }

  [[sfc_inline]] auto len() const -> usize {
    return _len;
  }

  [[sfc_inline]] auto is_empty() const -> bool {
    return _len == 0;
  }

  [[sfc_inline]] explicit operator bool() const {
    return _len != 0;
  }

  [[sfc_inline]] operator Slice<const T>() const {
    return {_ptr, _len};
  }

 public:
  [[sfc_inline]] auto get_unchecked(usize idx) const -> const T& {
    return _ptr[idx];
  }

  [[sfc_inline]] auto get_unchecked_mut(usize idx) -> T& {
    return _ptr[idx];
  }

  [[sfc_inline]] auto get_unchecked(Range<> ids) const -> Slice<const T> {
    return {_ptr + ids._start, ids.len()};
  }

  [[sfc_inline]] auto get_unchecked_mut(Range<> ids) -> Slice<T> {
    return {_ptr + ids._start, ids.len()};
  }

  [[sfc_inline]] auto operator[](usize idx) const -> const T& {
    assert_fmt(idx < _len, "Slice::[]: idx(={}) out of range(={})", idx, _len);
    return _ptr[idx];
  }

  [[sfc_inline]] auto operator[](usize idx) -> T& {
    assert_fmt(idx < _len, "Slice::[]: idx(={}) out of range(={})", idx, _len);
    return _ptr[idx];
  }

  [[sfc_inline]] auto operator[](Range<> ids) const -> Slice<const T> {
    ids = ids % _len;
    return {_ptr + ids._start, ids.len()};
  }

  [[sfc_inline]] auto operator[](Range<> ids) -> Slice<T> {
    ids = ids % _len;
    return {_ptr + ids._start, ids.len()};
  }

  [[sfc_inline]] auto split_at(usize idx) const -> tuple::Tuple<Slice<const T>, Slice<const T>> {
    idx = cmp::min(idx, _len);
    return {this->get_unchecked({0, idx}), this->get_unchecked({idx, _len})};
  }

  [[sfc_inline]] auto split_at_mut(usize idx) -> tuple::Tuple<Slice, Slice> {
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
    for (auto p = _ptr, e = p + _len; p != e; ++p) {
      *p = val;
    }
  }

  void fill_with(auto&& f) {
    for (auto p = _ptr, e = p + _len; p != e; ++p) {
      *p = f();
    }
  }

  void copy_from_slice(Slice<const T> src) {
    assert_fmt(_len == src._len, "Slice::copy_from_slice: self.len(={}) != src.len(={})", _len,
               src._len);

    ptr::copy_nonoverlapping(src._ptr, _ptr, _len);
  }

 public:
  using Iter = slice::Iter<const T>;
  auto iter() const -> Iter;

  using IterMut = slice::Iter<T>;
  auto iter_mut() -> IterMut;

  auto windows(usize n) const;
  auto windows_mut(usize n);

  auto truncks(usize n) const;
  auto truncks_mut(usize n);

  auto contains(const T& val) const -> bool {
    return this->iter().any([&](const auto& x) { return x == val; });
  }

  auto find(const auto& val) const -> Option<usize> {
    return this->iter().position([&](auto& x) { return val == x; });
  }

  auto rfind(const auto& val) const -> Option<usize> {
    return this->iter().position([&](auto& x) { return val == x; });
  }

  auto find_if(auto&& f) const -> Option<usize> {
    return this->iter().position(f);
  }

  auto rfind_if(auto&& f) const -> Option<usize> {
    return this->iter().rposition(f);
  }

  auto min() const -> T {
    assert_fmt(_len != 0, "Slice::min: empty");
    return this->iter().min();
  }

  auto max() const -> T {
    assert_fmt(_len != 0, "Slice::min: empty");
    return this->iter().max();
  }

  auto min_by_key(auto&& f) const -> T {
    assert_fmt(_len != 0, "Slice::min_by_key: empty");
    return this->iter().min_by_key(f);
  }

  auto max_by_key(auto&& f) const -> T {
    assert_fmt(_len != 0, "Slice::max_by_key: empty");
    return this->iter().max_by_key(f);
  }

  auto sum() const -> T {
    assert_fmt(_len != 0, "Slice::sum: empty");
    return this->iter().sum();
  }

  void fmt(auto& f) const {
    f.debug_list().entries(this->iter());
  }

 public:
  friend auto begin(const Slice& self) -> const T* {
    return self._ptr;
  }

  friend auto end(const Slice& self) -> const T* {
    return self._ptr + self._len;
  }

  friend auto begin(Slice& self) -> T* {
    return self._ptr;
  }

  friend auto end(Slice& self) -> T* {
    return self._ptr + self._len;
  }
};

}  // namespace sfc::slice
