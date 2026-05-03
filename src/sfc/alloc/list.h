#pragma once

#include "sfc/alloc/alloc.h"

namespace sfc::list {

static constexpr auto MAX_SIZE = num::max_value<usize>() / 4;

template <class T, class A = alloc::Global>
class [[nodiscard]] List {
  using Inn = RawBuf<T, A>;
  Inn _inn = {};
  usize _len = 0;

 public:
  List() noexcept = default;

  ~List() noexcept {
    this->clear();
  }

  List(List&& other) noexcept : _inn{mem::move(other._inn)}, _len{mem::take(other._len)} {}

  List& operator=(List&& other) noexcept {
    if (this != &other) {
      mem::swap(_inn, other._inn);
      mem::swap(_len, other._len);
    }
    return *this;
  }

  static auto with_capacity(usize capacity) noexcept -> List {
    auto res = List{};
    res.reserve(capacity);
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
    return _inn.ptr();
  }

  auto as_mut_ptr() noexcept -> T* {
    return _inn.ptr();
  }

  auto len() const noexcept -> usize {
    return _len;
  }

  auto capacity() const noexcept -> usize {
    return _inn.cap();
  }

  auto is_empty() const noexcept -> bool {
    return _len == 0;
  }

  auto as_slice() const noexcept -> Slice<const T> {
    return Slice<const T>{_inn.ptr(), _len};
  }

  auto as_mut_slice() noexcept -> Slice<T> {
    return Slice<T>{_inn.ptr(), _len};
  }

  auto as_bytes() const noexcept -> Slice<const u8> {
    static_assert(__is_trivially_copyable(T));
    return {reinterpret_cast<const u8*>(_inn.ptr()), _len * sizeof(T)};
  }

  auto as_mut_bytes() noexcept -> Slice<u8> {
    static_assert(__is_trivially_copyable(T));
    return {reinterpret_cast<u8*>(_inn.ptr()), _len * sizeof(T)};
  }

  void set_len(usize new_len) noexcept {
    _len = new_len;
  }

 public:
  auto get_unchecked(usize idx) const noexcept -> const T& {
    return _inn[idx];
  }

  auto get_unchecked_mut(usize idx) noexcept -> T& {
    return _inn[idx];
  }

  auto operator[](usize idx) const noexcept -> const T& {
    sfc::expect(idx < _len, "List::[]: idx(={}) out of ids(={})", idx, _len);
    return _inn[idx];
  }

  auto operator[](usize idx) noexcept -> T& {
    sfc::expect(idx < _len, "List::[]: idx(={}) out of ids(={})", idx, _len);
    return _inn[idx];
  }

  auto operator[](ops::Range ids) const noexcept -> Slice<const T> {
    return this->as_slice()[ids];
  }

  auto operator[](ops::Range ids) noexcept -> Slice<T> {
    return this->as_mut_slice()[ids];
  }

  auto spare_capacity_mut() noexcept -> Slice<T> {
    return Slice{_inn.ptr() + _len, _inn.cap() - _len};
  }

 public:
  auto push(T val) -> T& {
    if (_len == _inn.cap()) {
      this->reserve(1);
    }
    const auto end = _inn.ptr() + _len;
    ptr::write(end, static_cast<T&&>(val));
    _len += 1;
    return *end;
  }

  auto pop() -> Option<T> {
    if (_len == 0) {
      return {};
    }
    _len -= 1;
    return ptr::read(&_inn[_len]);
  }

  auto pop_if(auto&& f) -> Option<T> {
    if (_len == 0 || !f(_inn[_len - 1])) {
      return {};
    }
    return this->pop();
  }

  auto swap_remove(usize idx) noexcept -> T {
    sfc::expect(idx < _len, "List::swap_remove: idx({}) out of ids([0,{}))", idx, _len);

    _len -= 1;
    auto tmp = ptr::read(_inn.ptr() + _len);
    if (idx == _len) {
      return tmp;
    }
    return mem::replace(_inn[idx], static_cast<T&&>(tmp));
  }

  void truncate(usize len) noexcept {
    if (len >= _len) {
      return;
    }
    ptr::drop(_inn.ptr() + len, _len - len);
    _len = len;
  }

  void reserve(usize additional) noexcept {
    _inn.reserve(_len, additional);
  }

  void reserve_exact(usize additional) noexcept {
    _inn.reserve_exact(_len, additional);
  }

  void shrink_to(usize cap) noexcept {
    _inn.shrink_to(cap);
  }

  void shrink_to_fit() noexcept {
    _inn.shrink_to(_len);
  }

  void clear() noexcept {
    ptr::drop(_inn.ptr(), _len);
    _len = 0;
  }

  void insert(usize idx, T val) noexcept {
    sfc::expect(idx <= _len, "List::insert: idx(={}) out of ids([0,{}))", idx, _len);

    this->reserve(1);
    const auto tail_ptr = _inn.ptr() + idx;
    const auto tail_len = _len - idx;
    ptr::shift_elements_right(tail_ptr, tail_len, 1);
    ptr::write(_inn.ptr() + idx, static_cast<T&&>(val));
    _len += 1;
  }

  auto remove(usize idx) noexcept -> T {
    sfc::expect(idx < _len, "List::remove: idx(={}) out of ids([0,{}))", idx, _len);

    const auto hole_ptr = _inn.ptr() + idx;
    const auto tail_ptr = hole_ptr + 1;
    const auto tail_len = _len - idx - 1;
    auto res = ptr::read(hole_ptr);
    ptr::shift_elements_left(tail_ptr, tail_len, 1);
    _len -= 1;
    return res;
  }

  void drain(ops::Range ids) noexcept {
    ids = ids % _len;

    const auto hole_ptr = _inn.ptr() + ids.start;
    const auto hole_len = ids.len();
    const auto tail_ptr = hole_ptr + hole_len;
    const auto tail_len = _len - ids.end;
    ptr::drop(hole_ptr, hole_len);
    ptr::shift_elements_left(tail_ptr, tail_len, hole_len);
    _len -= ids.len();
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
    ptr::uninit_move(other._inn.ptr(), _inn.ptr() + _len, other._len);
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
    static_assert(__is_trivially_copyable(T));
    this->reserve(other._len);
    ptr::copy_nonoverlapping(other._ptr, _inn.ptr() + _len, other._len);
    _len += other._len;
  }

  void retain(auto&& f) noexcept {
    const auto idx = this->iter().position([&](const T& x) { return !f(x); }).unwrap_or(_len);
    if (idx == _len) {
      return;
    }

    auto cnt = idx;
    const auto ptr = _inn.ptr();
    for (auto i = idx + 1; i < _len; ++i) {
      if (f(ptr[i])) {
        ptr[cnt++] = static_cast<T&&>(ptr[i]);
      }
    }
    this->truncate(cnt);
  }

 public:
  using Iter = slice::Iter<const T>;
  auto iter() const noexcept -> Iter {
    return Slice{_inn.ptr(), _len}.iter();
  }

  using IterMut = slice::Iter<T>;
  auto iter_mut() noexcept -> IterMut {
    return Slice{_inn.ptr(), _len}.iter_mut();
  }

 public:
  // trait: Clone
  auto clone() const noexcept -> List {
    auto res = List{};
    res.extend_from_slice({_inn.ptr(), _len});
    return res;
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    return Slice{_inn.ptr(), _len}.fmt(f);
  }

  // trait: io::Write
  auto write(Slice<const u8> buf) -> io::Result<usize> {
    this->extend_from_slice(buf);
    return buf.len();
  }

  // trait: serde::Serialize
  auto serialize(auto& ser) const {
    const auto v = Slice{_inn.ptr(), _len};
    return v.serialize(ser);
  }

  // trait:: serde::Deserialize
  static auto deserialize(auto& des) {
    auto visit = [&](auto&& seq) { return seq.template collect<List, T>(); };
    return des.deserialize_seq(visit);
  }
};

}  // namespace sfc::list

namespace sfc {
using list::List;
}  // namespace sfc
