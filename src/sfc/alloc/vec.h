#pragma once

#include "sfc/alloc/alloc.h"

namespace sfc::vec {

static constexpr auto MAX_SIZE = num::max_value<usize>() >> 8U;

template <class T, class A = alloc::Global>
class RawVec {
  T* _ptr{nullptr};
  usize _cap{0};
  [[no_unique_address]] A _a{};

 public:
  [[gnu::always_inline]] RawVec() noexcept = default;

  [[gnu::always_inline]] ~RawVec() noexcept {
    if (!_ptr) return;

    const auto layout = alloc::Layout::template array<T>(_cap);
    _a.dealloc(_ptr, layout);
  }

  [[gnu::always_inline]] RawVec(RawVec&& other) noexcept
      : _ptr{mem::take(other._ptr)}, _cap{mem::take(other._cap)}, _a{mem::move(other._a)} {}

  [[gnu::always_inline]] RawVec& operator=(RawVec&& other) noexcept {
    if (this != &other) {
      mem::swap(_ptr, other._ptr);
      mem::swap(_cap, other._cap);
      mem::swap(_a, other._a);
    }
    return *this;
  }

  static auto with_capacity(usize capacity, A alloc = {}) noexcept -> RawVec {
    const auto layout = alloc::Layout::template array<T>(capacity);

    auto res = RawVec{};
    res._ptr = static_cast<T*>(alloc.alloc(layout));
    res._cap = capacity;
    res._a = alloc;
    return res;
  }

  [[gnu::always_inline]] auto ptr() const noexcept -> T* {
    return _ptr;
  }

  [[gnu::always_inline]] auto cap() const noexcept -> usize {
    return _cap;
  }

  [[gnu::always_inline]] auto operator[](usize idx) const noexcept -> const T& {
    return _ptr[idx];
  }

  [[gnu::always_inline]] auto operator[](usize idx) noexcept -> T& {
    return _ptr[idx];
  }

  auto allocator() -> A& {
    return _a;
  }

  void realloc(usize used, usize new_cap) noexcept {
    if (used > new_cap) {
      return;
    }

    const auto old_ptr = _ptr;
    const auto old_layout = alloc::Layout::template array<T>(_cap);
    const auto new_layout = alloc::Layout::template array<T>(new_cap);
    if (__is_trivially_copyable(T) || used == 0) {
      _ptr = static_cast<T*>(_a.realloc(old_ptr, old_layout, new_cap * sizeof(T)));
    } else {
      _ptr = static_cast<T*>(_a.alloc(new_layout));
      ptr::uninit_move(old_ptr, _ptr, used);
      _a.dealloc(old_ptr, old_layout);
    }
    _cap = new_cap;
  }
};

template <class T, class A = alloc::Global>
class [[nodiscard]] Vec {
  RawVec<T, A> _buf = {};
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
    return _buf.ptr();
  }

  auto as_mut_ptr() noexcept -> T* {
    return _buf.ptr();
  }

  auto len() const noexcept -> usize {
    return _len;
  }

  auto capacity() const noexcept -> usize {
    return _buf.cap();
  }

  auto is_empty() const noexcept -> bool {
    return _len == 0;
  }

  auto as_slice() const noexcept -> Slice<const T> {
    return Slice<const T>{_buf.ptr(), _len};
  }

  auto as_mut_slice() noexcept -> Slice<T> {
    return Slice<T>{_buf.ptr(), _len};
  }

  auto as_bytes() const noexcept -> Slice<const u8> {
    static_assert(__is_trivially_copyable(T));
    return {reinterpret_cast<const u8*>(_buf.ptr()), _len * sizeof(T)};
  }

  auto as_mut_bytes() noexcept -> Slice<u8> {
    static_assert(__is_trivially_copyable(T));
    return {reinterpret_cast<u8*>(_buf.ptr()), _len * sizeof(T)};
  }

  void set_len(usize new_len) noexcept {
    _len = new_len;
  }

 public:
  auto get_unchecked(usize idx) const noexcept -> const T& {
    return _buf[idx];
  }

  auto get_unchecked_mut(usize idx) noexcept -> T& {
    return _buf[idx];
  }

  auto operator[](usize idx) const noexcept -> const T& {
    sfc::expect(idx < _len, "Vec::[]: idx(={}) out of ids(={})", idx, _len);
    return _buf[idx];
  }

  auto operator[](usize idx) noexcept -> T& {
    sfc::expect(idx < _len, "Vec::[]: idx(={}) out of ids(={})", idx, _len);
    return _buf[idx];
  }

  auto operator[](ops::Range ids) const noexcept -> Slice<const T> {
    return this->as_slice()[ids];
  }

  auto operator[](ops::Range ids) noexcept -> Slice<T> {
    return this->as_mut_slice()[ids];
  }

  auto spare_capacity_mut() noexcept -> Slice<T> {
    return Slice{_buf.ptr() + _len, _buf.cap() - _len};
  }

 public:
  auto push(T val) -> T& {
    if (_len == _buf.cap()) {
      this->reserve(1);
    }
    const auto end = _buf.ptr() + _len;
    ptr::write(end, static_cast<T&&>(val));
    _len += 1;
    return *end;
  }

  auto pop() -> Option<T> {
    if (_len == 0) {
      return {};
    }
    _len -= 1;
    return ptr::read(&_buf[_len]);
  }

  auto pop_if(auto&& f) -> Option<T> {
    if (_len == 0 || !f(_buf[_len - 1])) {
      return {};
    }
    return this->pop();
  }

  auto swap_remove(usize idx) noexcept -> T {
    sfc::expect(idx < _len, "Vec::swap_remove: idx({}) out of ids([0,{}))", idx, _len);

    _len -= 1;
    auto tmp = ptr::read(_buf.ptr() + _len);
    if (idx == _len) {
      return tmp;
    }
    return mem::replace(_buf[idx], static_cast<T&&>(tmp));
  }

  void truncate(usize len) noexcept {
    if (len >= _len) {
      return;
    }
    ptr::drop_in_place(_buf.ptr() + len, _len - len);
    _len = len;
  }

  void reserve(usize additional) noexcept {
    if (additional <= _buf.cap() - _len) {
      return;
    }
    sfc::expect(additional < MAX_SIZE, "Vec::reserve: additional(={}) too large", additional);

    const auto req_cap = cmp::max(_len + additional, _buf.cap() * 2);
    const auto new_cap = cmp::max(req_cap, usize{8UL});
    sfc::expect(new_cap < MAX_SIZE, "Vec::reserve: required capacity(={}) too large", new_cap);
    _buf.realloc(_len, new_cap);
  }

  void reserve_exact(usize additional) noexcept {
    const auto new_cap = _len + additional;
    _buf.realloc(_len, new_cap);
  }

  void shrink_to(usize min_cap) noexcept {
    if (min_cap >= _buf.cap()) {
      return;
    }
    _buf.realloc(_len, min_cap);
  }

  void shrink_to_fit() noexcept {
    if (_len == _buf.cap()) {
      return;
    }
    _buf.realloc(_len, _len);
  }

  void clear() noexcept {
    ptr::drop_in_place(_buf.ptr(), _len);
    _len = 0;
  }

  void insert(usize idx, T val) noexcept {
    sfc::expect(idx <= _len, "Vec::insert: idx(={}) out of ids([0,{}))", idx, _len);

    this->reserve(1);
    const auto tail_ptr = _buf.ptr() + idx;
    const auto tail_len = _len - idx;
    ptr::shift_elements_right(tail_ptr, tail_len, 1);
    ptr::write(_buf.ptr() + idx, static_cast<T&&>(val));
    _len += 1;
  }

  auto remove(usize idx) noexcept -> T {
    sfc::expect(idx < _len, "Vec::remove: idx(={}) out of ids([0,{}))", idx, _len);

    const auto hole_ptr = _buf.ptr() + idx;
    const auto tail_ptr = hole_ptr + 1;
    const auto tail_len = _len - idx - 1;
    auto res = ptr::read(hole_ptr);
    ptr::shift_elements_left(tail_ptr, tail_len, 1);
    _len -= 1;
    return res;
  }

  void drain(ops::Range ids) noexcept {
    ids = ids % _len;

    const auto hole_ptr = _buf.ptr() + ids.start;
    const auto hole_len = ids.len();
    const auto tail_ptr = hole_ptr + hole_len;
    const auto tail_len = _len - ids.end;
    ptr::drop_in_place(hole_ptr, hole_len);
    ptr::shift_elements_left(tail_ptr, tail_len, hole_len);
    _len -= ids.len();
  }

  void resize(usize new_len, T value) noexcept {
    if (new_len <= _len) {
      return this->truncate(new_len);
    }
    this->extend_with(new_len - _len, value);
  }

  void append(Vec& other) noexcept {
    if (this == &other || other._len == 0) {
      return;
    }
    this->reserve(other._len);
    ptr::uninit_move(other._buf.ptr(), _buf.ptr() + _len, other._len);
    _len += other._len;
    other._len = 0;
  }

  void extend(auto&& iter) noexcept {
    if constexpr (requires { iter.len(); }) {
      this->reserve(iter.len());
    }
    iter.for_each([&](T val) { this->push(static_cast<T&&>(val)); });
  }

  void extend_with(usize cnt, const T& value) noexcept {
    this->reserve(cnt);
    for (auto idx = 0UL; idx < cnt; ++idx) {
      this->push(value);
    }
  }

  void extend_from_slice(Slice<const T> other) noexcept {
    this->reserve(other._len);
    ptr::uninit_copy(other._ptr, _buf.ptr() + _len, other._len);
    _len += other._len;
  }

  void retain(auto&& f) noexcept {
    auto pdst = _buf.ptr();
    auto pend = _buf.ptr() + _len;
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
    this->truncate(static_cast<usize>(pdst - _buf.ptr()));
  }

 public:
  using Iter = slice::Iter<const T>;
  using IterMut = slice::Iter<T>;

  auto iter() const noexcept -> Iter {
    return Slice{_buf.ptr(), _len}.iter();
  }

  auto iter_mut() noexcept -> IterMut {
    return Slice{_buf.ptr(), _len}.iter_mut();
  }

 public:
  // trait: Clone
  auto clone() const noexcept -> Vec {
    auto res = Vec{};
    res.extend_from_slice({_buf.ptr(), _len});
    return res;
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    return Slice{_buf.ptr(), _len}.fmt(f);
  }

  // trait: io::Write
  auto write(Slice<const u8> buf) -> io::Result<usize> {
    static_assert(same_<T, u8>);
    this->extend_from_slice(buf);
    return buf.len();
  }

  // trait: serde::Serialize
  auto serialize(auto& ser) const {
    const auto v = Slice{_buf.ptr(), _len};
    return v.serialize(ser);
  }

  // trait:: serde::Deserialize
  template <class D, class E = typename D::Error>
  static auto deserialize(D& des) -> Result<void, E> {
    auto res = Vec{};

    auto visit = [&](auto&& seq) -> Result<void, E> {
      while (seq.next()) {
        auto item = seq.template next_element<T>();
        if (item.is_err()) {
          return ~item;
        }
        res.push(mem::move(*item));
      }
      return {};
    };
    return des.deserialize_seq(visit);
  }
};

}  // namespace sfc::vec

namespace sfc {
using vec::RawVec;
using vec::Vec;
}  // namespace sfc
