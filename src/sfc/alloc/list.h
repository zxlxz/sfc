#pragma once

#include "sfc/alloc/rawbuf.h"

namespace sfc::list {

template <class T, class A = alloc::Global>
class [[nodiscard]] List {
  using Inn = RawBuf<T, A>;
  Inn _buf = {};
  usize _len = 0;

 public:
  explicit List() noexcept {}

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
    res._buf = Inn::with_capacity(capacity, alloc);
    return res;
  }

  static auto from(Slice<const T> s) -> List {
    auto res = List{};
    res.extend_from_slice(s);
    return res;
  }

  static auto from_iter(auto&& iter) -> List {
    auto res = List{};
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
  void set_len(usize new_len) noexcept {
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

    _len -= 1;
    auto tmp = ptr::read(_buf.ptr() + _len);
    if (idx == _len) {
      return tmp;
    }
    return mem::replace(_buf[idx], mem::move(tmp));
  }

  void truncate(usize len) noexcept {
    if (len >= _len) {
      return;
    }
    ptr::drop(_buf.ptr() + len, _len - len);
    _len = len;
  }

  void reserve(usize additional) noexcept {
    _buf.reserve(_len, additional);
  }

  void reserve_exact(usize additional) noexcept {
    _buf.reserve_exact(_len, additional);
  }

  void shrink_to(usize cap) noexcept {
    _buf.shrink_to(cap);
  }

  void shrink_to_fit() noexcept {
    _buf.shrink_to(_len);
  }

  void clear() noexcept {
    ptr::drop(_buf.ptr(), _len);
    _len = 0;
  }

  void insert(usize idx, T val) noexcept {
    sfc::assert_(idx <= _len, "List::insert: idx(={}) out of ids([0,{}))", idx, _len);

    this->reserve(1);
    const auto tail_ptr = _buf.ptr() + idx;
    const auto tail_len = _len - idx;
    ptr::shift_elements_right(tail_ptr, tail_len, 1);
    ptr::write(_buf.ptr() + idx, mem::move(val));
    _len += 1;
  }

  auto remove(usize idx) noexcept -> T {
    sfc::assert_(idx < _len, "List::remove: idx(={}) out of ids([0,{}))", idx, _len);

    const auto hole_ptr = _buf.ptr() + idx;
    const auto tail_ptr = hole_ptr + 1;
    const auto tail_len = _len - idx - 1;
    auto res = ptr::read(hole_ptr);
    ptr::shift_elements_left(tail_ptr, tail_len, 1);
    _len -= 1;
    return res;
  }

  class Drain;
  auto drain(ops::Range ids) noexcept -> Drain;

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
    _len += other._len;
    other._len = 0;
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

  void extend_from_slice(Slice<const T> other) noexcept {
    static_assert(__is_trivially_copyable(T));
    this->reserve(other._len);
    ptr::copy_nonoverlapping(other._ptr, _buf.ptr() + _len, other._len);
    _len += other._len;
  }

  void retain(auto&& f) noexcept {
    const auto idx = this->iter().position([&](const T& x) { return !f(x); }).unwrap_or(_len);
    if (idx == _len) {
      return;
    }

    auto cnt = idx;
    const auto ptr = _buf.ptr();
    for (auto i = idx + 1; i < _len; ++i) {
      if (f(ptr[i])) {
        ptr[cnt++] = mem::move(ptr[i]);
      }
    }
    this->truncate(cnt);
  }

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
  auto write(Slice<const u8> buf) -> io::Result<usize> {
    this->extend_from_slice(buf);
    return {buf.len()};
  }

  // trait: serde::Serialize
  auto serialize(auto& ser) const {
    const auto v = Slice{_buf.ptr(), _len};
    return v.serialize(ser);
  }

  // trait:: serde::Deserialize
  static auto deserialize(auto& des) {
    auto visit = [&](auto&& seq) { return seq.template collect<List, T>(); };
    return des.deserialize_seq(visit);
  }
};

template <class T, class A>
class List<T, A>::Drain {
  List& _list;
  Slice<T> _hole;

 public:
  Drain(List& list, ops::Range ids) noexcept : _list{list}, _hole{list[ids]} {}

  ~Drain() noexcept {
    ptr::drop(_hole._ptr, _hole._len);

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
auto List<T, A>::drain(ops::Range ids) noexcept -> Drain {
  return Drain{*this, ids};
}

}  // namespace sfc::list

namespace sfc {
using list::List;
}  // namespace sfc
