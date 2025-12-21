#pragma once

#include "sfc/alloc/alloc.h"
#include "sfc/io/mod.h"

namespace sfc::vec {

template <class T>
class RawVec {
 public:
  T* _ptr{nullptr};
  usize _cap{0};

 public:
  RawVec() noexcept = default;

  ~RawVec() noexcept {
    if (_ptr != nullptr) {
      __builtin_operator_delete(_ptr);
    }
  }

  RawVec(RawVec&& other) noexcept : _ptr{mem::take(other._ptr)}, _cap{mem::take(other._cap)} {}

  RawVec& operator=(RawVec&& other) noexcept {
    if (this != &other) {
      mem::swap(_ptr, other._ptr);
      mem::swap(_cap, other._cap);
    }
    return *this;
  }

  static auto with_capacity(usize capacity) noexcept -> RawVec {
    auto res = RawVec{};
    res.realloc(0, capacity);
    return res;
  }

  void realloc(usize used, usize new_cap) noexcept {
    if (new_cap == _cap || used > _cap || used > new_cap) {
      return;
    }
    const auto new_ptr = static_cast<T*>(new_cap ? __builtin_operator_new(new_cap * sizeof(T)) : nullptr);
    if (_ptr) {
      ptr::uninit_move(_ptr, new_ptr, used);
      __builtin_operator_delete(_ptr);
    }
    _ptr = new_ptr;
    _cap = new_cap;
  }
};

template <class T>
class [[nodiscard]] Vec {
  RawVec<T> _buf = {};
  usize _len = 0;

 public:
  Vec() noexcept = default;

  ~Vec() noexcept {
    this->clear();
  }

  Vec(Vec&& other) noexcept : _buf{mem::move(other._buf)}, _len{mem::take(other._len)} {}

  Vec& operator=(Vec&& other) noexcept {
    if (this != &other) {
      mem::swap(_buf, other._buf);
      mem::swap(_len, other._len);
    }
    return *this;
  }

  static auto with_capacity(usize capacity) noexcept -> Vec {
    auto res = Vec{};
    res.reserve(capacity);
    return res;
  }

  static auto from(Slice<const T> s) -> Vec {
    auto res = Vec{};
    res.extend_from_slice(s);
    return res;
  }

  static auto from_iter(auto&& iter) -> Vec {
    auto res = Vec{};
    res.extend(iter);
    return res;
  }

  auto as_ptr() const noexcept -> const T* {
    return _buf._ptr;
  }

  auto as_mut_ptr() noexcept -> T* {
    return _buf._ptr;
  }

  auto len() const noexcept -> usize {
    return _len;
  }

  auto capacity() const noexcept -> usize {
    return _buf._cap;
  }

  auto is_empty() const noexcept -> bool {
    return _len == 0;
  }

  auto as_slice() const noexcept -> Slice<const T> {
    return Slice<const T>{_buf._ptr, _len};
  }

  auto as_mut_slice() noexcept -> Slice<T> {
    return Slice<T>{_buf._ptr, _len};
  }

  void set_len(usize new_len) noexcept {
    if (new_len <= _buf._cap) {
      _len = new_len;
    }
  }

 public:
  auto get_unchecked(usize idx) const noexcept -> const T& {
    return _buf._ptr[idx];
  }

  auto get_unchecked_mut(usize idx) noexcept -> T& {
    return _buf._ptr[idx];
  }

  auto operator[](usize idx) const noexcept -> const T& {
    panicking::expect(idx < _len, "Vec::[]: idx(={}) out of ids(={})", idx, _len);
    return _buf._ptr[idx];
  }

  auto operator[](usize idx) noexcept -> T& {
    panicking::expect(idx < _len, "Vec::[]: idx(={}) out of ids(={})", idx, _len);
    return _buf._ptr[idx];
  }

  auto operator[](ops::Range ids) const noexcept -> Slice<const T> {
    return Slice<const T>{_buf._ptr, _len}[ids];
  }

  auto operator[](ops::Range ids) noexcept -> Slice<T> {
    return Slice<T>{_buf._ptr, _len}[ids];
  }

  auto spare_capacity_mut() noexcept -> Slice<T> {
    return Slice{_buf._ptr + _len, _buf._cap - _len};
  }

 public:
  auto push(T val) -> T& {
    if (_len == _buf._cap) {
      this->reserve(1);
    }
    const auto dst = _buf._ptr + _len;
    ptr::write(dst, static_cast<T&&>(val));
    _len += 1;
    return *dst;
  }

  auto pop() -> Option<T> {
    if (_len == 0) {
      return {};
    }

    _len -= 1;
    return ptr::read(&_buf._ptr[_len]);
  }

  auto pop_if(auto&& f) -> Option<T> {
    if (_len == 0 || !f(_buf._ptr[_len - 1])) {
      return {};
    }

    _len -= 1;
    return ptr::read(&_buf._ptr[_len]);
  }

  auto swap_remove(usize idx) noexcept -> T {
    panicking::expect(idx < _len, "Vec::swap_remove: idx({}) out of ids([0,{}))", idx, _len);

    _len -= 1;
    auto tmp = ptr::read(_buf._ptr + _len);
    if (idx == _len) {
      return tmp;
    }
    return mem::replace(_buf._ptr[idx], static_cast<T&&>(tmp));
  }

  void truncate(usize len) noexcept {
    if (len >= _len) {
      return;
    }

    ptr::drop_in_place(_buf._ptr + len, _len - len);
    _len = len;
  }

  void reserve(usize additional) noexcept {
    if (_len + additional <= _buf._cap) {
      return;
    }
    const auto new_cap = num::max(_buf._cap * 2, _len + additional);
    _buf.realloc(_len, new_cap);
  }

  void reserve_exact(usize additional) noexcept {
    if (_len + additional <= _buf._cap) {
      return;
    }
    const auto new_cap = _len + additional;
    _buf.realloc(_len, new_cap);
  }

  void shrink_to(usize min_cap) noexcept {
    if (min_cap < _len || min_cap >= _buf._cap) {
      return;
    }
    _buf.realloc(_len, min_cap);
  }

  void shrink_to_fit() noexcept {
    if (_len == _buf._cap) {
      return;
    }
    _buf.realloc(_len, _len);
  }

  void clear() noexcept {
    ptr::drop_in_place(_buf._ptr, _len);
    _len = 0;
  }

  void insert(usize idx, T val) noexcept {
    panicking::expect(idx <= _len, "Vec::insert: idx(={}) out of ids([0,{}))", idx, _len);

    this->reserve(1);
    ptr::shift_elements_right(_buf._ptr + idx, _len - idx, 1);
    ptr::write(_buf._ptr + idx, static_cast<T&&>(val));
    _len += 1;
  }

  auto remove(usize idx) noexcept -> T {
    panicking::expect(idx < _len, "Vec::remove: idx(={}) out of ids([0,{}))", idx, _len);

    auto dst = _buf._ptr + idx;
    auto res = ptr::read(dst);
    ptr::shift_elements_left(dst + 1, _len - idx - 1, 1);
    _len -= 1;
    return res;
  }

  void drain(ops::Range ids) noexcept {
    auto tmp = (*this)[ids];
    if (tmp._len == 0) {
      return;
    }
    const auto tmp_tail = tmp._ptr + tmp._len;
    const auto tail_len = _buf._ptr + _len - tmp_tail;
    ptr::shift_elements_left(tmp_tail, tail_len, tmp._len);
    _len -= tmp._len;
  }

  void resize(usize new_len, T value) noexcept {
    if (new_len <= _len) {
      return this->truncate(new_len);
    }

    this->extend_with(new_len - _len, value);
  }

  void append(Vec& other) noexcept {
    if (other._len == 0) {
      return;
    }

    this->reserve(other._len);
    ptr::uninit_move(other._buf._ptr, _buf._ptr + _len, other._len);
    _len += other._len;
    other._len = 0;
  }

  void extend(auto&& iter) noexcept {
    if constexpr (requires { iter.len(); }) {
      this->reserve(iter.len());
    }
    iter.for_each([&](T val) { this->push(static_cast<T&&>(val)); });
  }

  void extend_with(usize cnt, T value) noexcept {
    this->reserve(cnt);
    for (auto idx = 0UL; idx < cnt; ++idx) {
      this->push(static_cast<T&&>(value));
    }
  }

  void extend_from_slice(Slice<const T> other) noexcept {
    static_assert(trait::copy_<T>);
    this->reserve(other._len);
    ptr::uninit_copy(other._ptr, _buf._ptr + _len, other._len);
    _len += other._len;
  }

  void retain(auto&& f) noexcept {
    auto pdst = _buf._ptr;
    auto pend = _buf._ptr + _len;
    while (pdst != pend && f(*pdst)) {
      ++pdst;
    }
    if (pdst == pend) {
      return;
    }
    for (auto psrc = pdst + 1; psrc != pend; ++psrc) {
      if (f(*psrc)) {
        *pdst++ = static_cast<T&&>(*psrc);
      }
    }
    this->truncate(static_cast<usize>(pdst - _buf._ptr));
  }

 public:
  using Iter = slice::Iter<const T>;
  using IterMut = slice::Iter<T>;

  auto iter() const noexcept -> Iter {
    return Slice{_buf._ptr, _len}.iter();
  }

  auto iter_mut() noexcept -> IterMut {
    return Slice{_buf._ptr, _len}.iter_mut();
  }

  auto first() const noexcept -> Option<const T&> {
    if (_len == 0) {
      return {};
    }
    return _buf._ptr[0];
  }

  auto first_mut() noexcept -> Option<T&> {
    if (_len == 0) {
      return {};
    }
    return _buf._ptr[0];
  }

  auto last() const noexcept -> Option<const T&> {
    if (_len == 0) {
      return {};
    }
    return _buf._ptr[_len - 1];
  }

  auto last_mut() noexcept -> Option<T&> {
    if (_len == 0) {
      return {};
    }
    return _buf._ptr[_len - 1];
  }

 public:
  // trait: Deref<[const T]>
  auto operator*() const noexcept -> Slice<const T> {
    return Slice<const T>{_buf._ptr, _len};
  }

  // trait: Deref<[T]>
  auto operator*() noexcept -> Slice<T> {
    return Slice{_buf._ptr, _len};
  }

  // trait: Clone
  auto clone() const noexcept -> Vec {
    auto res = Vec{};
    res.extend_from_slice({_buf._ptr, _len});
    return res;
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    return Slice{_buf._ptr, _len}.fmt(f);
  }

  // trait: io::Write
  auto write(Slice<const u8> buf) -> io::Result<usize> {
    static_assert(__is_same(T, u8));
    this->extend_from_slice(buf);
    return buf.len();
  }

  // trait: serde::Serialize
  auto serialize(auto& ser) const {
    const auto v = Slice{_buf._ptr, _len};
    return v.serialize(ser);
  }

  // trait:: serde::Deserialize
  template <class D, class E = typename D::Error>
  static auto deserialize(D& des) -> Result<void, E> {
    auto res = Vec{};

    auto visit = [&](auto&& seq) -> Result<void, E> {
      while (seq.has_next()) {
        auto item = _TRY(seq.template next_element<T>());
        res.push(mem::move(item));
      }
      return {};
    };
    return des.deserialize_seq(visit);
  }
};

}  // namespace sfc::vec

namespace sfc {
using vec::Vec;
using vec::RawVec;
}  // namespace sfc
