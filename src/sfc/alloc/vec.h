#pragma once

#include "sfc/alloc/alloc.h"

namespace sfc::vec {

template <class T>
class Vec;

template <class T, class A = alloc::Global>
class [[nodiscard]] Buf {
  friend class Vec<T>;

  T* _ptr = nullptr;
  usize _cap = 0;
  A _a{};

 public:
  Buf() noexcept = default;

  ~Buf() {
    if (_ptr == nullptr) {
      return;
    }

    _a.dealloc_array(_ptr, _cap);
    _ptr = nullptr;
  }

  Buf(Buf&& other) noexcept
      : _ptr{mem::take(other._ptr)}, _cap{mem::take(other._cap)}, _a{mem::move(other._a)} {}

  auto operator=(Buf&& other) noexcept -> Buf& {
    auto tmp = static_cast<Buf&&>(*this);
    _ptr = mem::take(other._ptr);
    _cap = mem::take(other._cap);
    _a = mem::move(other._a);
    return *this;
  }

  static auto with_capacity(usize capacity) -> Buf {
    auto res = Buf{};
    res._ptr = res._a.template alloc_array<T>(capacity);
    res._cap = capacity;
    return res;
  }

  auto ptr() const -> T* {
    return _ptr;
  }

  auto capacity() const -> usize {
    return _cap;
  }

  auto operator[](usize idx) const -> const T& {
    return _ptr[idx];
  }

  auto operator[](usize idx) -> T& {
    return _ptr[idx];
  }

  void shrink_to(usize used, usize min_cap) {
    if (min_cap >= _cap) {
      return;
    }

    this->reserve_extract(used, min_cap - _cap);
  }

  void reserve(usize used, usize additional) {
    if (used + additional <= _cap) {
      return;
    }

    const auto new_cap = cmp::max(2 * _cap, used + additional);
    const auto new_add = cmp::max(usize{8U}, new_cap - used);
    this->reserve_extract(used, new_add);
  }

  void reserve_extract(usize used, usize additional) {
    if (used + additional <= _cap) {
      return;
    }

    const auto new_cap = used + additional;
    _ptr = _a.realloc_array(_ptr, _cap, new_cap);
    _cap = new_cap;
  }
};

template <class T>
class [[nodiscard]] Vec {
  Buf<T> _buf = {};
  usize _len = 0;

 public:
  Vec() noexcept = default;

  ~Vec() {
    this->clear();
  }

  Vec(Vec&& other) noexcept : _buf{mem::move(other._buf)}, _len{mem::take(other._len)} {}

  Vec& operator=(Vec&& other) noexcept {
    auto tmp = static_cast<Vec&&>(*this);
    _buf = mem::move(other._buf);
    _len = mem::take(other._len);
    return *this;
  }

  static auto with_capacity(usize capacity) -> Vec {
    auto res = Vec{};
    res._buf = Buf<T>::with_capacity(capacity);
    return res;
  }

  static auto from(const auto& v) -> Vec {
    auto res = Vec{};
    res.extend_from_slice(Slice<const T>{v});
    return res;
  }

  static auto from_iter(auto&& iter) -> Vec {
    auto res = Vec{};
    res.extend(iter);
    return res;
  }

  auto as_ptr() const -> const T* {
    return _buf._ptr;
  }

  auto as_mut_ptr() -> T* {
    return _buf._ptr;
  }

  auto len() const -> usize {
    return _len;
  }

  auto capacity() const -> usize {
    return _buf._cap;
  }

  auto is_empty() const -> bool {
    return _len == 0;
  }

  auto is_full() const -> bool {
    return _len == _buf._cap;
  }

  explicit operator bool() const {
    return _len != 0;
  }

  auto as_slice() const -> slice::Slice<const T> {
    return {_buf._ptr, _len};
  }

  auto as_mut_slice() -> slice::Slice<T> {
    return {_buf._ptr, _len};
  }

  void set_len(usize new_len) {
    new_len = cmp::min(new_len, _buf._cap);
    _len = new_len;
  }

  auto clone() const -> Vec {
    auto res = Vec::with_capacity(_len);
    res.extend(this->as_slice().iter());
    return res;
  }

  void fmt(auto& f) const {
    f.debug_list().entries(this->iter());
  }

 public:
  auto get_unchecked(usize idx) const -> const T& {
    return _buf._ptr[idx];
  }

  auto get_unchecked_mut(usize idx) -> T& {
    return _buf._ptr[idx];
  }

  auto operator[](usize idx) const -> const T& {
    panicking::assert(idx < _len, "Vec::[]: idx(={}) out of ids(={})", idx, _len);
    return _buf[idx];
  }

  auto operator[](usize idx) -> T& {
    panicking::assert(idx < _len, "Vec::[]: idx(={}) out of ids(={})", idx, _len);
    return _buf[idx];
  }

  auto operator[](Range ids) const -> slice::Slice<const T> {
    return this->as_slice()[ids];
  }

  auto operator[](Range ids) -> slice::Slice<T> {
    return this->as_mut_slice()[ids];
  }

  auto first() const -> const T& {
    panicking::assert(_len != 0, "Vec::first: vec is empty");
    return _buf[0];
  }

  auto last() const -> const T& {
    panicking::assert(_len != 0, "Vec::last: vec is empty");
    return _buf[_len - 1];
  }

  void fill(const T& val) {
    this->as_mut_slice().fill(val);
  }

  void fill_with(auto&& f) {
    this->as_mut_slice().fill_with(f);
  }

 public:
  void push(T val) {
    this->reserve(1);
    ptr::write(&_buf._ptr[_len], static_cast<T&&>(val));
    _len += 1;
  }

  auto pop() -> Option<T> {
    if (_len == 0) {
      return {};
    }

    auto res = ptr::read(&_buf._ptr[_len - 1]);
    _len -= 1;
    return Option{res};
  }

  void truncate(usize len) {
    if (len >= _len) {
      return;
    }

    ptr::drop_in_place(_buf._ptr + len, _len - len);
    _len = len;
  }

  void reserve(usize additional) {
    _buf.reserve(_len, additional);
  }

  void reserve_extract(usize additional) {
    _buf.reserve_extract(_len, additional);
  }

  void shrink_to(usize min_capacity) {
    _buf.shrink_to(_len, min_capacity);
  }

  void shrink_to_fit() {
    _buf.shrink_to(_len, _len);
  }

  void clear() {
    this->truncate(0);
  }

  // swap with last, then remove
  auto swap_remove(usize idx) -> T {
    panicking::assert(idx <= _len, "Vec::swap_remove: idx({}) out of ids([0,{}))", idx, _len);

    auto res = this->pop().unwrap();
    if (idx != _len - 1) {
      mem::swap(res, _buf._ptr[idx]);
    }
    return res;
  }

  void insert(usize idx, T element) {
    panicking::assert(idx <= _len, "Vec::remove: idx({}) out of ids([0,{}))", idx, _len);

    this->reserve(1);

    const auto hole = _buf._ptr + idx;

    if (idx == _len) {
      ptr::write(hole, static_cast<T&&>(element));
    } else if (_len != 0) {
      const auto tail = _buf._ptr + _len;
      ptr::uninit_move(tail - 1, tail, 1);
      ptr::move(hole, hole + 1, _len - idx - 1);
      *hole = static_cast<T&&>(element);
    }
    _len += 1;
  }

  auto remove(usize idx) -> T {
    panicking::assert(idx < _len, "Vec::remove: idx({}) out of ids([0,{}))", idx, _len);

    const auto res = mem::move(_buf[idx]);
    this->drain({idx, idx + 1});
    return res;
  }

  void drain(Range ids) {
    auto tmp = (*this)[ids];
    if (tmp.is_empty()) {
      return;
    }

    const auto dst = tmp._ptr;
    const auto src = dst + tmp._len;
    const auto end = _buf._ptr + _len;
    ptr::move(src, dst, static_cast<usize>(end - dst));
    this->truncate(_len - tmp._len);
  }

  void resize(usize new_len, T value) {
    if (new_len <= _len) {
      return this->truncate(new_len);
    }

    this->extend_with(new_len - _len, value);
  }

  void append(Vec& other) {
    this->reserve(other.len());

    ptr::uninit_move(other.as_mut_ptr(), this->as_mut_ptr() + _len, other.len());
    this->set_len(_len + other.len());
  }

  void extend(auto iter) {
    this->reserve(iter.len());
    iter.for_each([&](T val) { this->push(static_cast<T&&>(val)); });
  }

  void extend_with(usize cnt, T value) {
    this->reserve(cnt);

    for (auto idx = 0U; idx < cnt; ++idx) {
      this->push(value);
    }
  }

  void extend_from_slice(Slice<const T> other) {
    this->reserve(other._len);
    ptr::uninit_copy(other._ptr, _buf._ptr + _len, other._len);
    this->set_len(_len + other._len);
  }

  void retain(auto&& f) {
    auto pdst = _buf._ptr;
    for (auto psrc = _buf._ptr; psrc != _buf._ptr + _len; ++psrc) {
      if (!f(*psrc)) {
        continue;
      }
      if (psrc != pdst) {
        *pdst = static_cast<T&&>(*psrc);
      }
      ++pdst;
    }

    this->truncate(pdst - _buf._ptr);
  }

 public:
  auto iter() const -> slice::Iter<const T> {
    return this->as_slice().iter();
  }

  auto iter_mut() -> slice::Iter<T> {
    return this->as_mut_slice().iter_mut();
  }

  auto windows(usize n) const -> slice::Windows<const T> {
    return this->as_slice().windows(n);
  }

  auto windows_mut(usize n) -> slice::Windows<T> {
    return this->as_mut_slice().windows_mut(n);
  }

  auto truncks(usize n) const -> slice::Truncks<const T> {
    return this->as_slice().truncks(n);
  }

  auto truncks_mut(usize n) -> slice::Truncks<T> {
    return this->as_mut_slice().truncks_mut(n);
  }
};

template <class T>
auto begin(const Vec<T>& v) -> const T* {
  return v.as_ptr();
}

template <class T>
auto end(const Vec<T>& v) -> const T* {
  return v.as_ptr() + v.len();
}

template <class T>
auto begin(Vec<T>& v) -> T* {
  return v.as_mut_ptr();
}

template <class T>
auto end(Vec<T>& v) -> T* {
  return v.as_mut_ptr() + v.len();
}

}  // namespace sfc::vec
