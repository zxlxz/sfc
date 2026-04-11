#pragma once

#include "sfc/core/io.h"
#include "sfc/core/iter.h"
#include "sfc/core/ops.h"
#include "sfc/core/ptr.h"
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
  [[gnu::always_inline]] constexpr Slice() noexcept = default;

  [[gnu::always_inline]] constexpr Slice(T* ptr, usize len) noexcept : _ptr{ptr}, _len{len} {}

  template <usize N>
  [[gnu::always_inline]] constexpr Slice(T (&v)[N]) noexcept : _ptr{v}, _len{N} {}

  [[gnu::always_inline]] constexpr auto ptr() const noexcept -> T* {
    return _ptr;
  }

  [[gnu::always_inline]] constexpr auto len() const noexcept -> usize {
    return _len;
  }

  [[gnu::always_inline]] constexpr auto as_ptr() const noexcept -> const T* {
    return _ptr;
  }

  [[gnu::always_inline]] constexpr auto as_mut_ptr() noexcept -> T* {
    return _ptr;
  }

  [[gnu::always_inline]] constexpr auto is_empty() const noexcept -> bool {
    return _len == 0;
  }

  [[gnu::always_inline]] constexpr operator Slice<const T>() const noexcept {
    return {_ptr, _len};
  }

  [[gnu::always_inline]] auto as_bytes() const noexcept -> Slice<const u8> {
    static_assert(__is_trivially_copyable(T));
    return {reinterpret_cast<const u8*>(_ptr), _len * sizeof(T)};
  }

  [[gnu::always_inline]] auto as_mut_bytes() noexcept -> Slice<u8> {
    static_assert(__is_trivially_copyable(T));
    return {reinterpret_cast<u8*>(_ptr), _len * sizeof(T)};
  }

 public:
  [[gnu::always_inline]] auto operator[](usize idx) const noexcept -> const T& {
    sfc::expect(idx < _len, "Slice::[]: idx(={}) out of range(={})", idx, _len);
    return _ptr[idx];
  }

  [[gnu::always_inline]] auto operator[](usize idx) noexcept -> T& {
    sfc::expect(idx < _len, "Slice::[]: idx(={}) out of range(={})", idx, _len);
    return _ptr[idx];
  }

  auto operator[](ops::Range ids) const noexcept -> Slice<const T> {
    const auto end = ids.end < _len ? ids.end : _len;
    const auto len = ids.start < end ? end - ids.start : 0UL;
    return Slice<const T>{_ptr + ids.start, len};
  }

  auto operator[](ops::Range ids) noexcept -> Slice<T> {
    const auto end = ids.end < _len ? ids.end : _len;
    const auto len = ids.start < end ? end - ids.start : 0UL;
    return Slice<T>{_ptr + ids.start, len};
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
    sfc::expect(i < _len, "Slice::[]: i(={}), out of range(={})", i, _len);
    sfc::expect(j < _len, "Slice::[]: j(={}), out of range(={})", j, _len);

    if (i != j) {
      mem::swap(_ptr[i], _ptr[j]);
    }
  }

  void fill(T val) noexcept {
    for (auto p = _ptr, e = p + _len; p != e; ++p) {
      *p = val;
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
        return i;
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
  [[gnu::always_inline]] auto begin() const noexcept -> const T* {
    return _ptr;
  }

  [[gnu::always_inline]] auto begin() noexcept -> T* {
    return _ptr;
  }

  [[gnu::always_inline]] auto end() const noexcept -> const T* {
    return _ptr + _len;
  }

  [[gnu::always_inline]] auto end() noexcept -> T* {
    return _ptr + _len;
  }

  [[gnu::always_inline]] auto iter() const noexcept -> Iter<const T> {
    return {{}, _ptr, _ptr + _len};
  }

  [[gnu::always_inline]] auto iter_mut() noexcept -> Iter<T> {
    return {{}, _ptr, _ptr + _len};
  }

  [[gnu::always_inline]] auto windows(usize n) const noexcept -> Windows<const T> {
    return {{}, *this, n};
  }

  [[gnu::always_inline]] auto windows_mut(usize n) noexcept -> Windows<T> {
    return {{}, *this, n};
  }

  [[gnu::always_inline]] auto chunks(usize n) const noexcept -> Chunks<const T> {
    return {{}, *this, n};
  }

  [[gnu::always_inline]] auto chunks_mut(usize n) noexcept -> Chunks<T> {
    return {{}, *this, n};
  }

 public:
  // trait: ops::Eq
  constexpr auto operator==(Slice<const T> other) const noexcept -> bool {
    if (_len != other._len) return false;

    for (auto i = 0UL; i < _len; ++i) {
      if (_ptr[i] != other._ptr[i]) {
        return false;
      }
    }
    return true;
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    f.debug_list().entries(this->iter());
  }

  // trait: io::Read
  auto read(Slice<u8> buf) noexcept -> io::Result<usize> {
    static_assert(same_<const T, const u8>);
    if (_len == 0 || buf._len == 0) {
      return usize{0};
    }
    const auto amt = _len < buf._len ? _len : buf._len;
    ptr::copy_nonoverlapping(_ptr, buf._ptr, amt);
    _ptr += amt;
    _len -= amt;
    return amt;
  }

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

template <class T>
struct Iter : iter::Iterator<T&> {
  T* _ptr = nullptr;
  T* _end = nullptr;

 public:
  auto len() const noexcept -> usize {
    return _end - _ptr;
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
  Slice<T> _buf = {};
  usize _len = 0;

 public:
  auto next() noexcept -> Option<Slice<T>> {
    if (_buf._len < _len) return {};

    const auto res = Slice<T>{_buf._ptr, _len};
    _buf._ptr += 1;
    _buf._len -= 1;
    return res;
  }

  auto next_back() noexcept -> Option<Slice<T>> {
    if (_buf._len < _len) return {};

    const auto res = Slice<T>{_buf._ptr + _buf._len - _len, _len};
    _buf._len -= 1;
    return res;
  }
};

template <class T>
struct Chunks : iter::Iterator<Slice<T>> {
  Slice<T> _buf = {};
  usize _len = 0;

 public:
  auto next() noexcept -> Option<Slice<T>> {
    if (_buf._len == 0) return {};

    const auto pos = _len < _buf._len ? _len : _buf._len;
    const auto res = Slice<T>{_buf._ptr, pos};
    _buf._ptr += pos;
    _buf._len -= pos;
    return res;
  }
};

}  // namespace sfc::slice

namespace sfc {
using slice::Slice;
}  // namespace sfc
