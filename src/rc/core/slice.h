#pragma once

#include "rc/core/iter.h"
#include "rc/core/ops.h"
#include "rc/core/ptr.h"

namespace rc::slice {

constexpr inline usize $ = usize(-1);

template <class T>
struct Cursor {
  T* _ptr;

  auto operator++() noexcept -> void { ++_ptr; }
  auto operator--() noexcept -> void { --_ptr; }

  auto operator*() const noexcept -> T& { return *_ptr; }

  auto operator!=(const Cursor& other) const noexcept -> bool {
    return _ptr != other._ptr;
  }
};

template <class T>
struct Iter : iter::Iterator<Iter<T>> {
  using Item = T&;

  Cursor<T> _itr;
  Cursor<T> _end;

  constexpr Iter(T* itr, T* end) noexcept : _itr{itr}, _end{end} {}

  auto next() noexcept -> Option<T&> {
    if (_itr != _end) {
      return {*_itr._ptr++};
    }
    return {};
  }

  auto next_back() noexcept -> Option<T&> {
    if (_itr != _end) {
      return {*--_itr._ptr};
    }
    return {};
  }

  auto len() const noexcept -> usize { return usize(_end._ptr - _itr._ptr); }
};

#pragma region slice

template <class T>
struct Slice {
  using Item = T;

  using Iter = slice::Iter<const T>;
  using IterMut = slice::Iter<T>;
  using Cursor = slice::Cursor<const T>;
  using CursorMut = slice::Cursor<T>;

  T* _ptr;
  usize _len;

  constexpr Slice() noexcept : _ptr{nullptr}, _len{0} {}

  constexpr Slice(T* ptr, usize len) noexcept : _ptr{ptr}, _len{len} {}

  template <usize N>
  constexpr Slice(T (&v)[N]) noexcept : _ptr{v}, _len{N} {}

  constexpr operator Slice<const T>() const noexcept {
    return Slice<const T>{_ptr, _len};
  }

  constexpr auto as_ptr() const noexcept -> const T* { return _ptr; }
  constexpr auto as_mut_ptr() noexcept -> T* { return _ptr; }

  constexpr auto len() const noexcept -> usize { return _len; }
  constexpr auto is_empty() const noexcept -> bool { return _len == 0; }

  constexpr auto get_unchecked(usize idx) const noexcept -> const T& {
    return _ptr[idx];
  }

  constexpr auto get_unchecked_mut(usize idx) noexcept -> T& {
    return _ptr[idx];
  }

  constexpr auto operator[](usize idx) const -> const T& {
    rc::assert(idx < _len, u8"Slice::operator[]: out of range");
    return _ptr[idx];
  }

  constexpr auto operator[](usize idx) -> T& {
    rc::assert(idx < _len, u8"Slice::operator[]: out of range");
    return _ptr[idx];
  }

  constexpr auto slice_unchecked(usize start, usize end) const -> Slice {
    return Slice{_ptr + start, end - start};
  }

  constexpr auto slice(usize start, usize end) const -> Slice {
    start = ops::min(start, _len);
    end = ops::min(end, _len);
    if (start >= end) return Slice{nullptr, 0};
    return Slice{_ptr + start, end - start};
  }

  constexpr auto slice_from(usize start) const -> Slice {
    if (start >= _len) return Slice{nullptr, 0};
    return Slice{_ptr + start, _len - start};
  }

  constexpr auto split_at(usize mid) const noexcept -> Tuple<Slice, Slice> {
    rc::assert(mid < _len, u8"Slice::split_at(): out of range");
    const auto x = this->slice_unchecked(0, mid);
    const auto y = this->slice_unchecked(mid, _len);
    return Tuple{x, y};
  }

  auto iter() const noexcept -> Iter { return Iter{_ptr, _ptr + _len}; }
  auto iter_mut() noexcept -> IterMut { return IterMut{_ptr, _ptr + _len}; }

  auto begin() const noexcept -> Cursor { return Cursor{_ptr}; }
  auto end() const noexcept -> Cursor { return Cursor{_ptr + _len}; }

  auto begin() noexcept -> CursorMut { return CursorMut{_ptr}; }
  auto end() noexcept -> CursorMut { return CursorMut{_ptr + _len}; }

  auto eq(const Slice<const T>& other) const noexcept -> bool {
    if (_len != other._len) {
      return false;
    }
    return cmp::all_eq(_ptr, other._ptr, _len);
  }

  auto starts_with(const Slice<const T>& needle) const noexcept -> bool {
    if (needle._len == 0) return true;
    if (needle._len > _len) return false;
    return cmp::all_eq(_ptr, needle._ptr, needle._len);
  }

  auto ends_with(const Slice<const T>& needle) const noexcept -> bool {
    if (needle._len == 0) return true;
    if (needle._len > _len) return false;
    return cmp::all_eq(_ptr + _len - needle._len, needle._ptr, needle._len);
  }

  auto find(const T& val) const noexcept -> Option<usize> {
    return this->iter().find(val);
  }

  auto rfind(const T& val) const noexcept -> Option<usize> {
    return this->iter().rfind(val);
  }

  auto contains(const T& val) const noexcept -> bool {
    return this->iter().contains(val);
  }
};

template <class T>
Slice(T*, usize)->Slice<T>;

template <class T, usize N>
Slice(T (&)[N])->Slice<T>;
#pragma endregion

}  // namespace rc::slice

namespace rc {
using slice::Slice;
}  // namespace rc
