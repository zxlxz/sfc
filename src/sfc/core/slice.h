#pragma once

#include "sfc/core/iter.h"
#include "sfc/core/mem.h"
#include "sfc/core/ops.h"
#include "sfc/core/option.h"
#include "sfc/core/ptr.h"
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
  T* _ptr = nullptr;
  usize _len = 0;

 public:
  constexpr Slice() noexcept = default;

  constexpr Slice(T* ptr, usize len) noexcept : _ptr{ptr}, _len{len} {}

  template <usize N>
  constexpr Slice(T (&v)[N]) noexcept : _ptr{v}, _len{N} {}

  auto as_ptr() const noexcept -> const T* {
    return _ptr;
  }

  auto as_mut_ptr() noexcept -> T* {
    return _ptr;
  }

  auto len() const noexcept -> usize {
    return _len;
  }

  auto is_empty() const noexcept -> bool {
    return _len == 0;
  }

  explicit operator bool() const noexcept {
    return _len != 0;
  }

 public:
  auto get_unchecked(usize idx) const noexcept -> const T& {
    return _ptr[idx];
  }

  auto get_unchecked_mut(usize idx) noexcept -> T& {
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

  auto operator[](ops::Range ids) const noexcept -> Slice<const T> {
    const auto end = ids._end < _len ? ids._end : _len;
    const auto pos = ids._start < end ? ids._start : end;
    const auto len = pos <= end ? end - pos : 0U;
    return {_ptr + pos, len};
  }

  auto operator[](ops::Range ids) noexcept -> Slice<T> {
    const auto end = ids._end < _len ? ids._end : _len;
    const auto pos = ids._start < end ? ids._start : end;
    const auto len = pos <= end ? end - pos : 0U;
    return {_ptr + pos, len};
  }

  auto split_at(usize mid) const noexcept -> tuple::Tuple<Slice<const T>, Slice<const T>> {
    const auto x = mid < _len ? mid : _len;
    return tuple::Tuple{Slice<const T>{_ptr, x}, Slice<const T>{_ptr + x, _len - x}};
  }

  auto split_at_mut(usize mid) noexcept -> tuple::Tuple<Slice, Slice> {
    const auto x = mid < _len ? mid : _len;
    return tuple::Tuple{Slice{_ptr, x}, Slice{_ptr + x, _len - x}};
  }

 public:
  void swap(usize i, usize j) {
    panicking::assert(i < _len, "Slice::[]: i(={}), out of range(={})", i, _len);
    panicking::assert(j < _len, "Slice::[]: j(={}), out of range(={})", j, _len);

    if (i != j) {
      mem::swap(_ptr[i], _ptr[j]);
    }
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
  auto operator==(Slice<const T> other) const -> bool {
    if (_len != other._len) {
      return false;
    }
    for (auto i = 0U; i < _len; ++i) {
      if (_ptr[i] != other._ptr[i]) {
        return false;
      }
    }
    return true;
  }

  auto find(const T& x) const noexcept -> option::Option<usize> {
    for (auto i = usize{0UL}; i < _len; ++i) {
      if (_ptr[i] == x) {
        return i;
      }
    }
    return {};
  }

  auto rfind(const T& x) const noexcept -> option::Option<usize> {
    for (auto i = _len; i > 0; --i) {
      if (_ptr[i - 1] == x) {
        return i - 1;
      }
    }
    return {};
  }

  auto contains(const T& x) const noexcept -> bool {
    for (auto i = 0U; i < _len; ++i) {
      if (_ptr[i] == x) {
        return true;
      }
    }
    return false;
  }

  auto starts_with(Slice<const T> needle) const noexcept -> bool {
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
  auto iter() const noexcept -> Iter<const T> {
    return Iter<const T>{_ptr, _ptr + _len};
  }

  auto iter_mut() noexcept -> Iter<T> {
    return Iter<T>{_ptr, _ptr + _len};
  }

  auto windows(usize n) const noexcept -> Windows<const T> {
    return {*this, n};
  }

  auto windows_mut(usize n) noexcept -> Windows<T> {
    return {*this, n};
  }

  auto truncks(usize n) const noexcept -> Truncks<const T> {
    return {*this, n};
  }

  auto truncks_mut(usize n) noexcept -> Truncks<T> {
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
  Iter(T* _ptr, T* _end) noexcept : _ptr{_ptr}, _end{_end} {}

  [[nodiscard]] auto len() const -> usize {
    const auto res = _end - _ptr;
    return res >= 0 ? static_cast<usize>(res) : 0U;
  }

  auto next() -> option::Option<T&> {
    if (_ptr >= _end) {
      return {};
    }
    return *_ptr++;
  }

  auto next_back() -> option::Option<T&> {
    if (_ptr >= _end) {
      return {};
    }
    return *--_end;
  }
};

template <class T>
struct Windows : iter::Iterator<Windows<T>, Slice<T>> {
  Slice<T> _buf;
  usize _len;

 public:
  explicit operator bool() const {
    return _buf._len >= _len;
  }

  auto next() -> option::Option<Slice<T>> {
    if (_buf._len < _len) {
      return {};
    }

    auto res = Slice<T>{_buf._ptr, _len};
    _buf._ptr += 1;
    _buf._len -= 1;
    return res;
  }

  auto next_back() -> option::Option<Slice<T>> {
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
  usize _len;

 public:
  explicit operator bool() const {
    return _buf._len >= _len;
  }

  auto next() -> option::Option<Slice<T>> {
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
