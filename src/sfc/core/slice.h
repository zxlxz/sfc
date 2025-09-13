#pragma once

#include "sfc/core/ptr.h"
#include "sfc/core/iter.h"
#include "sfc/core/result.h"
#include "sfc/core/tuple.h"

namespace sfc::slice {

template <class T>
struct Iter;

template <class T>
struct Windows;

template <class T>
struct Chunks;

template <class T>
struct Slice {
  T* _ptr = nullptr;
  usize _len = 0;

 public:
  constexpr Slice() noexcept = default;

  constexpr Slice(T* ptr, usize len) noexcept : _ptr{ptr}, _len{len} {}

  template <usize N>
  constexpr Slice(T (&v)[N]) noexcept : _ptr{v}, _len{N} {}

  template <class U>
  constexpr Slice(const Slice<U>& v) noexcept : _ptr{v._ptr}, _len{v._len} {}

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
    panicking::expect(idx < _len, "Slice::[]: idx(={}) out of range(={})", idx, _len);
    return _ptr[idx];
  }

  auto operator[](usize idx) -> T& {
    panicking::expect(idx < _len, "Slice::[]: idx(={}) out of range(={})", idx, _len);
    return _ptr[idx];
  }

  auto slice_mut(usize start, usize end = static_cast<usize>(-1)) noexcept -> Slice<const T> {
    end = end < _len ? end : _len;
    return {_ptr + start, start < end ? end - start : 0U};
  }

  auto slice(usize start, usize end = static_cast<usize>(-1)) const noexcept -> Slice<T> {
    end = end < _len ? end : _len;
    return {_ptr + start, start < end ? end - start : 0U};
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
    panicking::expect(i < _len, "Slice::[]: i(={}), out of range(={})", i, _len);
    panicking::expect(j < _len, "Slice::[]: j(={}), out of range(={})", j, _len);

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
    static_assert(__is_trivially_copyable(T));

    if (_len != src._len) {
      panicking::panic("Slice::copy_from_slice: self.len(={}) != src.len(={})", _len, src._len);
    }

    ptr::copy_nonoverlapping(src._ptr, _ptr, _len);
  }

  template <class U>
  auto read(Slice<U> buf) -> usize {
    static_assert(__is_same(const T, const U));

    const auto amt = _len < buf._len ? _len : buf._len;
    ptr::copy_nonoverlapping(_ptr, buf._ptr, amt);
    _ptr += amt;
    _len -= amt;
    return amt;
  }

 public:
  auto operator==(Slice<const T> other) const -> bool {
    if (_len != other._len) {
      return false;
    }
    for (auto i = 0UL; i < _len; ++i) {
      if (_ptr[i] != other._ptr[i]) {
        return false;
      }
    }
    return true;
  }

  auto find(const auto& x) const noexcept -> option::Option<usize> {
    for (auto i = 0UL; i < _len; ++i) {
      if (_ptr[i] == x) {
        return i;
      }
    }
    return {};
  }

  auto rfind(const auto& x) const noexcept -> option::Option<usize> {
    for (auto i = _len; i > 0; --i) {
      if (_ptr[i - 1] == x) {
        return i - 1;
      }
    }
    return {};
  }

  auto contains(const T& x) const noexcept -> bool {
    for (auto i = 0UL; i < _len; ++i) {
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
    return needle == Slice<const T>{_ptr, needle._len};
  }

  auto ends_with(Slice<const T> needle) const -> bool {
    if (_len < needle._len) {
      return false;
    }
    return needle == Slice<const T>{_ptr + _len - needle._len, needle._len};
  }

 public:
  auto begin() const noexcept -> const T* {
    return _ptr;
  }

  auto begin() noexcept -> T* {
    return _ptr;
  }

  auto end() const noexcept -> const T* {
    return _ptr + _len;
  }

  auto end() noexcept -> T* {
    return _ptr + _len;
  }

  auto iter() const noexcept -> Iter<const T> {
    return Iter<const T>{{}, _ptr, _ptr + _len};
  }

  auto iter_mut() noexcept -> Iter<T> {
    return Iter<T>{{}, _ptr, _ptr + _len};
  }

  auto windows(usize n) const noexcept -> Windows<const T> {
    return Windows{{}, *this, n};
  }

  auto windows_mut(usize n) noexcept -> Windows<T> {
    return Windows{{}, *this, n};
  }

  auto chunks(usize n) const noexcept -> Chunks<const T> {
    return Chunks{{}, *this, n};
  }

  auto chunks_mut(usize n) noexcept -> Chunks<T> {
    return Chunks{{}, *this, n};
  }

 public:
  auto as_bytes_mut() -> Slice<u8> {
    return Slice{reinterpret_cast<u8*>(_ptr), _len * sizeof(T)};
  }

  void fmt(auto& f) const {
    auto imp = f.debug_list();
    for (auto& x : *this) {
      imp.entry(x);
    }
  }

  auto serialize(auto& s) const {
    return s.ser_list(*this);
  }
};

template <class T, usize N>
Slice(T (&)[N]) -> Slice<T>;

template <class T>
Slice(T*, auto) -> Slice<T>;

template <class T>
struct Iter : iter::Iterator<T&> {
  T* _ptr = nullptr;
  T* _end = nullptr;

 public:
  operator bool() const noexcept {
    return _ptr < _end;
  }

  auto len() const -> usize {
    return _ptr < _end ? static_cast<usize>(_end - _ptr) : 0U;
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
struct Windows : iter::Iterator<Slice<T>> {
  Slice<T> _buf = {};
  usize _len = 0;

 public:
  explicit operator bool() const {
    return _buf._len >= _len;
  }

  auto next() -> option::Option<Slice<T>> {
    if (_buf._len < _len) {
      return {};
    }

    auto res = Slice{_buf._ptr, _len};
    _buf._ptr += 1;
    _buf._len -= 1;
    return res;
  }

  auto next_back() -> option::Option<Slice<T>> {
    if (_buf._len < _len) {
      return {};
    }

    auto res = Slice{_buf._ptr + _buf._len - _len, _len};
    _buf._len -= 1;
    return res;
  }
};

template <class T>
struct Chunks : iter::Iterator<Slice<T>> {
  Slice<T> _buf = {};
  usize _len = 0;

 public:
  explicit operator bool() const {
    return _buf._len >= _len;
  }

  auto next() -> option::Option<Slice<T>> {
    if (_buf._len == 0) {
      return {};
    }
    const auto pos = _len < _buf._len ? _len : _buf._len;
    const auto res = Slice<T>{_buf._ptr, pos};
    _buf._ptr += pos;
    _buf._len -= pos;
    return res;
  }
};

}  // namespace sfc::slice
