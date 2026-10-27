#pragma once

#include "sfc/alloc/raw_buf.h"

namespace sfc::list {

using slice::Range;

template <class T, class A = alloc::Global>
class [[nodiscard]] List {
  using Buf = raw_buf::RawBuf<T, A>;
  Buf _buf{};
  usize _len{0};

 public:
  List() noexcept {}

  ~List() noexcept {
    this->clear();
  }

  List(List&& other) noexcept : _buf{mem::move(other._buf)}, _len{mem::take(other._len)} {}

  List& operator=(List&& other) noexcept {
    if (this != &other) {
      mem::swap(_buf, other._buf);
      mem::swap(_len, other._len);
    }
    return *this;
  }

  static auto with_capacity(usize capacity, A alloc = {}) noexcept -> List {
    auto res = List{};
    res._buf = Buf::with_capacity(capacity, alloc);
    return res;
  }

  static auto from(Slice<const T> s) -> List {
    auto res = List{};
    res.extend_from_slice(s);
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
    return {_buf.ptr(), _len};
  }

  auto as_mut_slice() noexcept -> Slice<T> {
    return {_buf.ptr(), _len};
  }

 public:
  auto first() const noexcept -> Option<const T&> {
    return this->as_slice().first();
  }

  auto first_mut() noexcept -> Option<T&> {
    return this->as_mut_slice().first();
  }

  auto last() const noexcept -> Option<const T&> {
    return this->as_slice().last();
  }

  auto last_mut() noexcept -> Option<T&> {
    return this->as_mut_slice().last();
  }

  auto get_unchecked(usize idx) const noexcept -> const T& {
    return _buf[idx];
  }

  auto get_unchecked_mut(usize idx) noexcept -> T& {
    return _buf[idx];
  }

  auto operator[](usize idx) const noexcept -> const T& {
    sfc::assert_(idx < _len, "List::[]: idx(={}) out of ids(={})", idx, _len);
    return _buf[idx];
  }

  auto operator[](usize idx) noexcept -> T& {
    sfc::assert_(idx < _len, "List::[]: idx(={}) out of ids(={})", idx, _len);
    return _buf[idx];
  }

  auto operator[](Range ids) const noexcept -> Slice<const T> {
    return this->as_slice()[ids];
  }

  auto operator[](Range ids) noexcept -> Slice<T> {
    return this->as_mut_slice()[ids];
  }

  auto spare_capacity_mut() noexcept -> Slice<T> {
    return Slice{_buf.ptr() + _len, _buf.cap() - _len};
  }

 public:
  void set_len(usize new_len) noexcept {
    const auto cap = _buf.cap();
    sfc::assert_(new_len <= cap, "List::set_len: new_len(={}) out of capacity(={})", new_len, cap);
    _len = new_len;
  }

  auto push(T val) -> T& {
    if (_len == _buf.cap()) {
      this->reserve(1);
    }
    const auto end = _buf.ptr() + _len;
    ptr::write(end, mem::move(val));
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
    sfc::assert_(idx < _len, "List::swap_remove: idx({}) out of ids([0,{}))", idx, _len);

    const auto base_ptr = _buf.ptr();
    auto value = ptr::read(&base_ptr[idx]);
    ptr::copy(&base_ptr[_len - 1], &base_ptr[idx], 1);
    _len -= 1;
    return value;
  }

  void truncate(usize len) noexcept {
    if (len >= _len) {
      return;
    }

    const auto remaining_len = _len - len;
    this->_len = len;
    ptr::drop_in_place(&_buf[len], remaining_len);
  }

  void reserve(usize additional) noexcept {
    _buf.reserve(_len, additional);
  }

  void reserve_exact(usize additional) noexcept {
    _buf.reserve_exact(_len, additional);
  }

  void shrink_to(usize min_cap) noexcept {
    _buf.shrink_to_fit(min_cap);
  }

  void shrink_to_fit() noexcept {
    _buf.shrink_to_fit(_len);
  }

  void clear() noexcept {
    auto elems = Slice{_buf.ptr(), _len};
    _len = 0;
    ptr::drop_in_place(elems._ptr, elems._len);
  }

  auto insert(usize idx, T val) noexcept -> T& {
    sfc::assert_(idx <= _len, "List::insert: idx(={}) out of ids([0,{}))", idx, _len);
    this->reserve(1);

    const auto p = _buf.ptr() + idx;
    if (idx < _len) {
      ptr::copy(p, p + 1, _len - idx);
    }
    ptr::write(p, mem::move(val));
    this->set_len(_len + 1);
    return *p;
  }

  auto remove(usize idx) noexcept -> T {
    sfc::assert_(idx < _len, "List::remove: idx(={}) out of ids([0,{}))", idx, _len);

    const auto p = _buf.ptr() + idx;
    auto ret = ptr::read(p);
    ptr::copy(p + 1, p, _len - idx - 1);
    this->set_len(_len - 1);
    return ret;
  }

  void resize(usize new_len, T value) noexcept {
    if (new_len <= _len) {
      return this->truncate(new_len);
    }
    this->extend_with(new_len - _len, value);
  }

  void append(List& other) noexcept {
    if (this == &other || other._len == 0) {
      return;
    }
    this->reserve(other._len);
    ptr::copy_nonoverlapping(other._buf.ptr(), _buf.ptr() + _len, other._len);
    this->set_len(_len + other._len);
    other.set_len(0);
  }

  void extend(auto&& iter) noexcept {
    if constexpr (requires { iter.len(); }) {
      this->reserve(iter.len());
    }
    iter.for_each([&](T val) { this->push(mem::move(val)); });
  }

  void extend_with(usize cnt, const T& value) noexcept {
    this->reserve(cnt);
    for (auto idx = 0UL; idx < cnt; ++idx) {
      this->push(value);
    }
  }

  /// clones and appends all elements in a slice to the List
  /// @note: user must ensure that the slice does not overlap with the list's buffer.
  void extend_from_slice(Slice<const T> other) noexcept {
    this->reserve(other._len);
    ptr::copy_nonoverlapping(other._ptr, _buf.ptr() + _len, other._len);
    this->set_len(_len + other._len);
  }

  void retain(auto&& f) noexcept {
    const auto p = _buf.ptr();
    auto read = usize{0};
    auto write = usize{0};
    while (read < _len) {
      if (f(p[read])) {
        if (write != read) {
          ptr::copy_nonoverlapping(p + read, p + write, 1);
        }
        write += 1;
      } else {
        ptr::drop_in_place(p + read, 1);
      }
      read += 1;
    }
    this->set_len(write);
  }

  class Drain;
  auto drain(Range ids) noexcept -> Drain;

 public:
  using Iter = slice::Iter<const T>;
  auto iter() const noexcept -> Iter {
    return Slice{_buf.ptr(), _len}.iter();
  }

  using IterMut = slice::Iter<T>;
  auto iter_mut() noexcept -> IterMut {
    return Slice{_buf.ptr(), _len}.iter_mut();
  }

 public:
  // trait: Clone
  auto clone() const noexcept -> List {
    auto res = List{};
    res.extend_from_slice({_buf.ptr(), _len});
    return res;
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    return Slice{_buf.ptr(), _len}.fmt(f);
  }

  // trait: io::Write
  auto write(Slice<const u8> buf) -> io::Result<usize> requires(trait::same_<T, u8>) {
    this->extend_from_slice(buf);
    return {buf.len()};
  }

  // trait: io::Write
  auto flush() -> io::Result<> requires(trait::same_<T, u8>) {
    return Ok{};
  }

  // trait: serde::Serialize
  auto serialize(auto& ser) const {
    const auto v = Slice{_buf.ptr(), _len};
    return v.serialize(ser);
  }

  // trait:: serde::Deserialize
  template <class D>
  static auto deserialize(D& des) -> D::template Result<List> {
    auto res = List{};

    auto imp = _TRY(des.deserialize_seq());
    _TRY(imp.template for_each<T>([&](T val) { res.push(mem::move(val)); }));
    return res;
  }

 public:
  // trait: ranged-for
  friend auto begin(const List& self) -> const T* {
    return self.as_ptr();
  }

  // trait: ranged-for
  friend auto end(const List& self) -> const T* {
    return self.as_ptr() + self.len();
  }

  // trait: ranged-for
  friend auto begin(List& self) -> T* {
    return self.as_mut_ptr();
  }

  // trait: ranged-for
  friend auto end(List& self) -> T* {
    return self.as_mut_ptr() + self.len();
  }
};

template <class T, class A>
class List<T, A>::Drain {
  List& _list;
  Slice<T> _hole;

 public:
  Drain(List& list, Range range) noexcept : _list{list}, _hole{list[range]} {}

  ~Drain() noexcept {
    ptr::drop_in_place(_hole._ptr, _hole._len);

    const auto tail_ptr = _hole._ptr + _hole._len;
    const auto tail_pos = num::cast_unsigned(tail_ptr - _list.as_ptr());
    const auto tail_len = _list._len - tail_pos;
    ptr::shift_elements_left(tail_ptr, tail_len, _hole._len);
    _list._len -= _hole._len;
  }

  Drain(const Drain&) = delete;
  Drain& operator=(const Drain&) = delete;

 public:
  void for_each(auto&& f) noexcept {
    for (auto& x : _hole) {
      f(x);
    }
  }
};

template <class T, class A>
auto List<T, A>::drain(Range ids) noexcept -> Drain {
  return Drain{*this, ids};
}

}  // namespace sfc::list

namespace sfc {
using list::List;
}  // namespace sfc
