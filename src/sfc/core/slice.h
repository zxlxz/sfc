#pragma once

#include "sfc/core/ops.h"
#include "sfc/core/ptr.h"
#include "sfc/core/iter.h"
#include "sfc/core/result.h"
#include "sfc/core/tuple.h"
#include "sfc/io/mod.h"

namespace sfc::slice {

template <class T>
struct Iter;

template <class T>
struct Windows;

template <class T>
struct Chunks;

template <class T>
struct Slice {
  using Item = T;

  T* _ptr = nullptr;
  usize _len = 0;

 public:
  [[gnu::always_inline]] constexpr Slice() noexcept = default;

  [[gnu::always_inline]] constexpr Slice(T* ptr, usize len) noexcept : _ptr{ptr}, _len{len} {}

  template <usize N>
  [[gnu::always_inline]] constexpr Slice(T (&v)[N]) noexcept : _ptr{v}, _len{N} {}

  [[gnu::always_inline]] auto as_ptr() const noexcept -> const T* {
    return _ptr;
  }

  [[gnu::always_inline]] auto as_mut_ptr() noexcept -> T* {
    return _ptr;
  }

  [[gnu::always_inline]] auto len() const noexcept -> usize {
    return _len;
  }

  [[gnu::always_inline]] auto is_empty() const noexcept -> bool {
    return _len == 0;
  }

  [[gnu::always_inline]] explicit operator bool() const noexcept {
    return _len != 0;
  }

  [[gnu::always_inline]] operator Slice<const T>() const noexcept {
    return {_ptr, _len};
  }

 public:
  auto operator[](usize idx) const noexcept -> const T& {
    panicking::expect(idx < _len, "Slice::[]: idx(={}) out of range(={})", idx, _len);
    return _ptr[idx];
  }

  auto operator[](usize idx) noexcept -> T& {
    panicking::expect(idx < _len, "Slice::[]: idx(={}) out of range(={})", idx, _len);
    return _ptr[idx];
  }

  auto operator[](ops::Range ids) const noexcept -> Slice<const T> {
    const auto start = ids.start;
    const auto end = ids.end < _len ? ids.end : _len;
    return Slice<const T>{_ptr + start, start < end ? end - start : 0U};
  }

  auto operator[](ops::Range ids) noexcept -> Slice<T> {
    const auto start = ids.start;
    const auto end = ids.end < _len ? ids.end : _len;
    return Slice<T>{_ptr + start, start < end ? end - start : 0U};
  }

  auto split_at(usize mid) const noexcept -> Tuple<Slice<const T>, Slice<const T>> {
    const auto x = mid < _len ? mid : _len;
    return {Slice<const T>{_ptr, x}, Slice<const T>{_ptr + x, _len - x}};
  }

  auto split_at_mut(usize mid) noexcept -> Tuple<Slice, Slice> {
    const auto x = mid < _len ? mid : _len;
    return {Slice{_ptr, x}, Slice{_ptr + x, _len - x}};
  }

 public:
  void swap(usize i, usize j) noexcept {
    panicking::expect(i < _len, "Slice::[]: i(={}), out of range(={})", i, _len);
    panicking::expect(j < _len, "Slice::[]: j(={}), out of range(={})", j, _len);

    if (i != j) {
      mem::swap(_ptr[i], _ptr[j]);
    }
  }

  void fill(T val) noexcept {
    if constexpr (trait::tv_copy_<T> && sizeof(T) == 1) {
      __builtin_memset(_ptr, val, _len);
    } else {
      for (auto p = _ptr, e = p + _len; p != e; ++p) {
        *p = val;
      }
    }
  }

  void copy_from_slice(Slice<const T> src) noexcept {
    static_assert(__is_trivially_copyable(T));
    panicking::expect(src._ptr != nullptr, "Slice::copy_from_slice: src.ptr == nullptr");

    ptr::copy_nonoverlapping(src._ptr, _ptr, _len);
  }

 public:
  auto operator==(Slice<const T> other) const noexcept -> bool {
    if (_len != other._len) {
      return false;
    }
    if (_ptr == other._ptr) {
      return true;
    }
    if constexpr (trait::tv_copy_<T> && sizeof(T) == 1) {
      return __builtin_memcmp(_ptr, other._ptr, _len) == 0;
    } else {
      for (auto i = 0UL; i < _len; ++i) {
        if (_ptr[i] != other._ptr[i]) {
          return false;
        }
      }
      return true;
    }
  }

  auto contains(const T& x) const noexcept -> bool {
    if (_len == 0) {
      return {};
    }
    if constexpr (trait::tv_copy_<T> && sizeof(T) == 1) {
      return __builtin_memchr(_ptr, x, _len) != nullptr;
    } else {
      for (auto i = 0UL; i < _len; ++i) {
        if (_ptr[i] == x) {
          return true;
        }
      }
    }
    return false;
  }

  auto find(const T& x) const noexcept -> Option<usize> {
    if (_len == 0) {
      return {};
    }
    if constexpr (trait::tv_copy_<T> && sizeof(T) == 1) {
      const auto p = __builtin_memchr(_ptr, x, _len);
      if (p != nullptr) {
        return static_cast<usize>(static_cast<const T*>(p) - _ptr);
      }
    } else {
      for (auto i = 0UL; i < _len; ++i) {
        if (_ptr[i] == x) {
          return i;
        }
      }
    }
    return {};
  }

  auto starts_with(Slice<const T> needle) const noexcept -> bool {
    if (_len < needle._len) {
      return false;
    }
    return needle == Slice{_ptr, needle._len};
  }

  auto ends_with(Slice<const T> needle) const -> bool {
    if (_len < needle._len) {
      return false;
    }
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
    return {{}, _ptr, _ptr + _len};
  }

  auto iter_mut() noexcept -> Iter<T> {
    return {{}, _ptr, _ptr + _len};
  }

  auto windows(usize n) const noexcept -> Windows<const T> {
    return {{}, *this, n};
  }

  auto windows_mut(usize n) noexcept -> Windows<T> {
    return {{}, *this, n};
  }

  auto chunks(usize n) const noexcept -> Chunks<const T> {
    return {{}, *this, n};
  }

  auto chunks_mut(usize n) noexcept -> Chunks<T> {
    return {{}, *this, n};
  }

 public:
  // trait: fmt::Display
  void fmt(auto& f) const {
    auto imp = f.debug_list();
    for (auto& x : *this) {
      imp.entry(x);
    }
  }

  // trait: io::Read
  auto read(Slice<u8> buf) noexcept -> io::Result<usize> {
    static_assert(trait::same_<const T, const u8>);
    if (_len == 0 || buf._len == 0) {
      return 0;
    }
    const auto amt = _len < buf._len ? _len : buf._len;
    __builtin_memcpy(buf._ptr, _ptr, amt);
    _ptr += amt;
    _len -= amt;
    return amt;
  }

  // trait: serde::Serialize
  auto as_bytes() const noexcept -> Slice<const u8> {
    static_assert(trait::tv_copy_<T>);
    return {reinterpret_cast<const u8*>(_ptr), _len * sizeof(T)};
  }

  // trait: serde::Serialize
  void serialize(auto& ser) const {
    auto imp = ser.serialize_seq();
    for (auto i = 0; i < _len; ++i) {
      imp.serialize_element(_ptr[i]);
    }
  }
};

template <class T, usize N>
Slice(T (&)[N]) -> Slice<T>;

template <class T>
Slice(T*, auto) -> Slice<T>;

template <class T>
struct Iter : iter::Iterator<T&> {
  using Item = T&;
  T* _ptr = nullptr;
  T* _end = nullptr;

 public:
  operator bool() const noexcept {
    return _ptr < _end;
  }

  auto len() const noexcept -> usize {
    return _ptr < _end ? static_cast<usize>(_end - _ptr) : 0U;
  }

  auto next() noexcept -> Option<T&> {
    if (_ptr >= _end) {
      return {};
    }
    return *_ptr++;
  }

  auto next_back() noexcept -> Option<T&> {
    if (_ptr >= _end) {
      return {};
    }
    return *--_end;
  }
};

template <class T>
struct Windows : iter::Iterator<Slice<T>> {
  using Item = Slice<T>;

  Slice<T> _buf = {};
  usize _len = 0;

 public:
  explicit operator bool() const noexcept {
    return _buf._len >= _len;
  }

  auto next() noexcept -> Option<Item> {
    if (_buf._len < _len) {
      return {};
    }

    auto res = Item{_buf._ptr, _len};
    _buf._ptr += 1;
    _buf._len -= 1;
    return res;
  }

  auto next_back() noexcept -> Option<Item> {
    if (_buf._len < _len) {
      return {};
    }

    auto res = Item{_buf._ptr + _buf._len - _len, _len};
    _buf._len -= 1;
    return res;
  }
};

template <class T>
struct Chunks : iter::Iterator<Slice<T>> {
  using Item = Slice<T>;

  Slice<T> _buf = {};
  usize _len = 0;

 public:
  explicit operator bool() const noexcept {
    return _buf._len >= _len;
  }

  auto next() noexcept -> Option<Item> {
    if (_buf._len == 0) {
      return {};
    }
    const auto pos = _len < _buf._len ? _len : _buf._len;
    const auto res = Item{_buf._ptr, pos};
    _buf._ptr += pos;
    _buf._len -= pos;
    return res;
  }
};

}  // namespace sfc::slice

namespace sfc::option {
template <class T>
struct Inner<slice::Slice<T>> {
  slice::Slice<T> _val;

 public:
  [[gnu::always_inline]] auto is_some() const noexcept -> bool {
    return _val._ptr != nullptr;
  }

  [[gnu::always_inline]] auto is_none() const noexcept -> bool {
    return _val._ptr == nullptr;
  }

  [[gnu::always_inline]] auto operator*() const noexcept -> const slice::Slice<T>& {
    return _val;
  }

  [[gnu::always_inline]] auto operator*() noexcept -> slice::Slice<T>& {
    return _val;
  }
};

}  // namespace sfc::option

namespace sfc {
using slice::Slice;
}  // namespace sfc
