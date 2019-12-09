#pragma once

#include "rc/alloc/mod.h"

namespace rc::vec {

template <class T>
struct RawVec {
  static constexpr usize MASK = 16 - 1;

  T* _ptr;
  usize _cap;

  constexpr RawVec() noexcept : _ptr{nullptr}, _cap{0} {}

  constexpr RawVec(T* ptr, usize cap) noexcept : _ptr{ptr}, _cap{cap} {}

  RawVec(RawVec&& other) noexcept : _ptr{other._ptr}, _cap{other._cap} {
    other._ptr = nullptr;
  }

  ~RawVec() {
    if (_ptr != nullptr) {
      alloc::dealloc(_ptr, _cap);
    }
  }

  static auto with_capacity(usize capacity) -> RawVec {
    const auto pointer = alloc::alloc<T>(capacity);
    return {pointer, capacity};
  }

  auto capacity() const noexcept -> usize { return _cap; }

  auto shrink_to_fit(usize amount) noexcept -> void {
    if (amount == 0) {
      alloc::dealloc(_ptr, _cap);
      _cap = 0;
      return;
    }
    if (_cap != amount) {
      _ptr = alloc::realloc(_ptr, _cap, amount);
      _cap = amount;
    }
  }

  auto reserve(usize used_capacity, usize needed_extra_capacity) -> void {
    const auto new_ext = cmp::max(_cap, (needed_extra_capacity | MASK) + 1);
    this->reserve_exact(used_capacity, new_ext);
  }

  auto reserve_exact(usize used_capacity, usize needed_extra_capacity) -> void {
    const auto new_cap = _cap + needed_extra_capacity;
    const auto new_ptr = alloc::realloc(_ptr, used_capacity, new_cap);
    _ptr = new_ptr;
    _cap = new_cap;
  }
};

template <class T>
struct Vec {
  using Iter = typename Slice<T>::Iter;
  using IterMut = typename Slice<T>::IterMut;
  using Cursor = typename Slice<T>::Cursor;
  using CursorMut = typename Slice<T>::CursorMut;
  
  RawVec<T> _buf;
  usize _len;

  constexpr Vec() noexcept : _buf{}, _len{0} {}

  constexpr Vec(RawVec<T>&& buf, usize len) noexcept
      : _buf{static_cast<RawVec<T>&&>(buf)}, _len{len} {}

  Vec(Vec&&) noexcept = default;

  ~Vec() { ptr::drop_n(_buf._ptr, _len); }

  static auto with_capacity(usize capacity) -> Vec {
    return {RawVec<T>::with_capacity(capacity), 0};
  }

  operator Slice<T>() noexcept { return {_buf._ptr, _len}; } 
  operator Slice<const T>() const noexcept { return {_buf._ptr, _len}; } 

  auto as_ptr() const noexcept -> const T* { return _buf._ptr; }
  auto as_mut_ptr() noexcept -> T* { return _buf._ptr; }

  auto capacity() const noexcept -> usize { return _buf._cap; }

  auto len() const noexcept -> usize { return _len; }
  auto is_empty() const noexcept -> bool { return _len == 0; }

  auto as_slice() const noexcept -> Slice<const T> { return {_buf._ptr, _len}; }
  auto as_mut_slice() noexcept -> Slice<T> { return {_buf._ptr, _len}; }

  auto iter() const noexcept -> Iter { return Iter{_buf._ptr, _len}; }
  auto iter_mut() noexcept -> IterMut { return IterMut{_buf._ptr, _len}; }

  auto begin() const noexcept -> Cursor { return Cursor{_buf._ptr}; }
  auto end() const noexcept -> Cursor { return Cursor{_buf._ptr + _len}; }

  auto begin() noexcept -> CursorMut { return CursorMut{_buf._ptr}; }
  auto end() noexcept -> CursorMut { return CursorMut{_buf._ptr + _len}; }

  auto operator[](usize idx) const -> const T& {
    rc::assert(idx < _len, u8"rc::vec: index out of range");
    return _buf._ptr[idx];
  }

  auto operator[](usize idx) -> T& {
    rc::assert(idx < _len, u8"rc::vec: index out of range");
    return _buf._ptr[idx];
  }

  auto slice(usize start, usize end) const noexcept -> Slice<T> {
    return this->as_slice().slice(start, end);
  }

  auto slice_mut(usize start, usize end) noexcept -> Slice<T> {
    return this->as_mut_slice().slice(start, end);
  }

  auto slice_from(usize start) const noexcept -> Slice<T> {
    return this->as_slice().slice_from(start);
  }

  auto clear() noexcept -> void {
    if (_len != 0) {
      ptr::drop_n(_buf._ptr, _len);
    }
    _len = 0;
  }

  auto truncate(usize new_len) noexcept -> void {
    if (new_len < _len) {
      ptr::drop_n(_buf._ptr + new_len, _len - new_len);
    }
    _len = new_len;
  }

  auto reserve(usize additional) noexcept -> void {
    if (_len + additional <= _buf._cap) return;
    _buf.reserve(_len, additional);
  }

  auto shrink_to_fit() noexcept -> void {
    if (_len == _buf._cap) return;
    if (_len != 0) _buf.shrink_to_fit(_len);
    _buf._cap = _len;
  }

  auto swap_remove(usize idx) -> T {
    rc::assert(idx < _len, u8"rc::vec::Vec: index out of range");

    const auto hole = _buf._ptr + idx;
    const auto last = _buf._ptr + _len - 1;
    --_len;
    return ptr::replace(hole, ptr::read(last));
  }

  auto insert(usize idx, T val) -> void {
    rc::assert(idx <= _len, u8"rc::vec::Vec: index out of range");

    if (_len == _buf._cap) {
      this->reserve(1);
    }
    ptr::mshr(_buf._ptr + idx, 1, _len - idx);
    ptr::write(&_buf._ptr[idx], static_cast<T&&>(val));
    _len += 1;
  }

  auto remove(usize idx) -> T {
    auto res = ptr::read(&_buf._ptr[idx]);
    ptr::mshl(_buf._ptr + idx + 1, 1, _len - idx - 1);
    _len -= 1;
    return res;
  }

  auto push(T val) noexcept -> void {
    if (_len == _buf._cap) {
      _buf.reserve(_len, 1);
    }
    ptr::write(_buf._ptr + _len, static_cast<T&&>(val));
    _len += 1;
  }

  auto pop() noexcept -> T {
    if (_len == 0) {
      rc::panic(u8"rc::vec::Vec: is empty");
    }
    return ptr::read(&_buf._ptr[--_len]);
  }

  auto resize(usize new_len, T val) -> void {
    if (new_len <= _len) {
      this->truncate(new_len);
    } else {
      this->extend(new_len - _len, static_cast<T&&>(val));
    }
  }

  template <class F>
  auto resize_with(usize new_len, F fun) -> void {
    if (new_len <= _len) {
      this->truncate(new_len);
    } else {
      this->extend_with(new_len - _len, static_cast<F&&>(fun));
    }
  }

  auto append(Vec& other) noexcept -> void {
    this->append_elements(other.as_slice());
    other._len = 0;
  }

  auto extend(usize n, const T& val) noexcept -> void {
    if (_len + n > _buf._cap) {
      _buf.reserve(_len, n);
    }
    for (T *p = _buf._ptr + _len, *q = p + n; p != q; ++p) {
      ptr::write(p, val);
    }
  }

  template <class F>
  auto extend_with(usize n, F fun) noexcept -> void {
    if (_len + n > _buf._cap) {
      _buf.reserve(_len, n);
    }
    for (T *p = _buf._ptr + _len, *q = p + n; p != q; ++p) {
      ptr::write(p, fun());
    }
  }

  auto append_elements(Slice<const T> buf) noexcept -> void {
    if (buf.is_empty()) return;

    this->reserve(buf._len);
    ptr::copy(buf._ptr, _buf._ptr + _len, buf._len);
    _len += buf._len;
  }

  auto write(Slice<const T> buf) -> usize {
    this->append_elements(buf);
    return buf._len;
  }
};

}  // namespace rc::vec

namespace rc {
using vec::Vec;
}
