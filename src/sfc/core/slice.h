#pragma once

#include "sfc/core/io.h"
#include "sfc/core/ops.h"
#include "sfc/core/mem.h"
#include "sfc/core/ptr.h"
#include "sfc/core/test.h"
#include "sfc/core/iter.h"
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

  constexpr auto ptr() const noexcept -> T* {
    return _ptr;
  }

  constexpr auto len() const noexcept -> usize {
    return _len;
  }

  constexpr auto as_ptr() const noexcept -> const T* {
    return _ptr;
  }

  constexpr auto as_mut_ptr() noexcept -> T* {
    return _ptr;
  }

  constexpr auto is_empty() const noexcept -> bool {
    return _len == 0;
  }

  auto as_bytes() const noexcept -> Slice<const u8> {
    static_assert(__is_trivially_copyable(T));
    return {ptr::cast<const u8>(_ptr), _len * sizeof(T)};
  }

  auto as_mut_bytes() noexcept -> Slice<u8> {
    static_assert(__is_trivially_copyable(T));
    return {ptr::cast<u8>(_ptr), _len * sizeof(T)};
  }

  operator Slice<const T>() const noexcept {
    return {_ptr, _len};
  }

 public:
  auto operator[](usize idx) const noexcept -> const T& {
    sfc::assert_(idx < _len, "Slice::[]: idx(={}) out of range(={})", idx, _len);
    return _ptr[idx];
  }

  auto operator[](usize idx) noexcept -> T& {
    sfc::assert_(idx < _len, "Slice::[]: idx(={}) out of range(={})", idx, _len);
    return _ptr[idx];
  }

  auto operator[](ops::Range ids) const noexcept -> Slice<const T> {
    ids = ids.wrap(_len);
    return Slice<const T>{_ptr + ids.start, ids.len()};
  }

  auto operator[](ops::Range ids) noexcept -> Slice<T> {
    ids = ids.wrap(_len);
    return Slice<T>{_ptr + ids.start, ids.len()};
  }

  auto split_at(usize mid) const noexcept -> Tuple<Slice<const T>, Slice<const T>> {
    const auto x = mid < _len ? mid : _len;
    return Tuple{Slice<const T>{_ptr, x}, Slice<const T>{_ptr + x, _len - x}};
  }

  auto split_at_mut(usize mid) noexcept -> Tuple<Slice, Slice> {
    const auto x = mid < _len ? mid : _len;
    return Tuple{Slice{_ptr, x}, Slice{_ptr + x, _len - x}};
  }

 public:
  void swap(usize i, usize j) noexcept {
    sfc::assert_(i < _len, "Slice::[]: i(={}), out of range(={})", i, _len);
    sfc::assert_(j < _len, "Slice::[]: j(={}), out of range(={})", j, _len);

    if (i != j) {
      mem::swap(_ptr[i], _ptr[j]);
    }
  }

  void fill(T val) noexcept {
    if constexpr (sizeof(T) == 1) {
      ptr::write_bytes(_ptr, val, _len);
    } else {
      for (auto p = _ptr, e = p + _len; p != e; ++p) {
        *p = val;
      }
    }
  }

  void copy_from_slice(Slice<const T> src) noexcept {
    static_assert(__is_trivially_copyable(T));
    ptr::copy_nonoverlapping(src._ptr, _ptr, _len);
  }

 public:
  auto contains(const T& x) const noexcept -> bool {
    for (auto i = 0UL; i < _len; ++i) {
      if (_ptr[i] == x) {
        return true;
      }
    }
    return false;
  }

  auto find(const T& x) const noexcept -> Option<usize> {
    for (auto i = 0UL; i != _len; ++i) {
      if (_ptr[i] == x) {
        return i;
      }
    }
    return {};
  }

  auto rfind(const T& x) const noexcept -> Option<usize> {
    for (auto i = _len; i != 0U; --i) {
      if (_ptr[i - 1] == x) {
        return i - 1;
      }
    }
    return {};
  }

  auto starts_with(Slice<const T> needle) const noexcept -> bool {
    if (_len < needle._len) return false;
    return needle == Slice{_ptr, needle._len};
  }

  auto ends_with(Slice<const T> needle) const -> bool {
    if (_len < needle._len) return false;
    return needle == Slice{_ptr + _len - needle._len, needle._len};
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
    return {_ptr, _len};
  }

  auto iter_mut() noexcept -> Iter<T> {
    return {_ptr, _len};
  }

  auto windows(usize n) const noexcept -> Windows<const T> {
    return {*this, n};
  }

  auto windows_mut(usize n) noexcept -> Windows<T> {
    return {*this, n};
  }

  auto chunks(usize n) const noexcept -> Chunks<const T> {
    return {*this, n};
  }

  auto chunks_mut(usize n) noexcept -> Chunks<T> {
    return {*this, n};
  }

 public:
  // trait: fmt::Display
  void fmt(auto& f) const {
    f.debug_list().entries(this->iter());
  }

  // trait: io::Read
  auto read(Slice<u8> buf) noexcept -> io::Result<usize>;

  // trait: serde::Serialize
  void serialize(auto& ser) const {
    auto imp = ser.serialize_seq();
    for (const auto& t : *this) {
      imp.serialize_element(t);
    }
  }
};

template <class T, usize N>
Slice(T (&)[N]) -> Slice<T>;

template <class T>
Slice(T*, usize) -> Slice<T>;

// ops::foreach
template <class T>
auto begin(Slice<T> v) -> const T* {
  return v._ptr;
}

template <class T>
auto end(Slice<T> v) -> const T* {
  return v._ptr + v._len;
}

// trait: ops::Eq
template <class A, class B>
constexpr auto operator==(const Slice<A>& a, const Slice<B>& b) noexcept -> bool {
  if (a._len != b._len) return false;

  for (auto i = 0UL; i < a._len; ++i) {
    if (a._ptr[i] != b._ptr[i]) {
      return false;
    }
  }
  return true;
}

template <class T>
struct Iter : iter::Iterator<T&> {
  T* _ptr = nullptr;
  T* _end = nullptr;

 public:
  Iter(T* ptr, usize len) noexcept : _ptr{ptr}, _end{ptr + len} {}

  auto len() const noexcept -> usize {
    return num::cast_unsigned(_end - _ptr);
  }

  auto next() noexcept -> Option<T&> {
    if (_ptr >= _end) return {};
    return *_ptr++;
  }

  auto next_back() noexcept -> Option<T&> {
    if (_ptr >= _end) return {};
    return *--_end;
  }
};

template <class T>
struct Windows : iter::Iterator<Slice<T>> {
  using Item = Slice<T>;

  T* _ptr;
  T* _end;
  usize _len;

 public:
  Windows(Slice<T> v, usize len) noexcept : _ptr{v._ptr}, _end{v._ptr + v._len}, _len{len} {}

  auto next() noexcept -> Option<Item> {
    if (_ptr + _len > _end) return {};
    _ptr += 1;
    return Item{_ptr - 1, _len};
  }

  auto next_back() noexcept -> Option<Item> {
    if (_ptr + _len > _end) return {};
    _end -= 1;
    return Item{_end - _len + 1, _len};
  }
};

template <class T>
struct Chunks : iter::Iterator<Slice<T>> {
  using Item = Slice<T>;
  T* _ptr;
  T* _end;
  usize _len;

 public:
  Chunks(Slice<T> v, usize len) noexcept : _ptr{v._ptr}, _end{v._ptr + v._len}, _len{len} {}

  auto next() noexcept -> Option<Item> {
    if (_ptr >= _end) return {};
    const auto cnt = _ptr + _len <= _end ? _len : num::cast_unsigned(_end - _ptr);
    _ptr += cnt;
    return Item{_ptr - cnt, cnt};
  }
};

}  // namespace sfc::slice

namespace sfc {
using slice::Slice;
}  // namespace sfc
