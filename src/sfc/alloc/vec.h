#pragma once

#include "alloc.h"

namespace sfc::vec {

template <class T>
class Vec;

template <class T, class A = alloc::Global>
class Buf {
  friend class Vec<T>;

  T* _ptr = nullptr;
  usize _cap = 0;
  [[no_unique_address]] A _a{};

  Buf(T* ptr, usize cap) noexcept : _ptr{ptr}, _cap{cap} {}

 public:
  Buf() noexcept = default;

  ~Buf() {
    if (_ptr == nullptr) {
      return;
    }

    _a.dealloc_array(_ptr, _cap);
    _ptr = nullptr;
  }

  Buf(Buf&& other) noexcept : _ptr{other._ptr}, _cap{other._cap} {
    other._ptr = nullptr;
    other._cap = 0;
  }

  auto operator=(Buf&& other) noexcept -> Buf& {
    auto tmp = static_cast<Buf&&>(other);
    this->swap(tmp);
    return *this;
  }

  static auto with_capacity(usize capacity) -> Buf {
    auto a = A{};
    const auto ptr = a.template alloc_array<T>(capacity);
    return Buf{static_cast<T*>(ptr), capacity};
  }

  [[sfc_inline]] void swap(Buf& dst) {
    mem::swap(_ptr, dst._ptr);
    mem::swap(_cap, dst._cap);
  }

  [[sfc_inline]] auto ptr() const -> T* {
    return _ptr;
  }

  [[sfc_inline]] auto capacity() const -> usize {
    return _cap;
  }

  [[sfc_inline]] auto operator[](usize idx) const -> const T& {
    return _ptr[idx];
  }

  [[sfc_inline]] auto operator[](usize idx) -> T& {
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

  [[sfc_inline]] Vec(Buf<T> buf, usize len) noexcept
      : _buf{static_cast<Buf<T>&&>(buf)}, _len{len} {}

 public:
  Vec() = default;

  ~Vec() {
    this->clear();
  }

  Vec(Vec&& other) noexcept
      : _buf{static_cast<Buf<T>&&>(other._buf)}, _len{mem::take(other._len)} {}

  Vec& operator=(Vec&& other) noexcept {
    auto tmp = static_cast<Vec&&>(other);
    this->swap(tmp);
    return *this;
  }

  void swap(Vec& other) noexcept {
    mem::swap(_buf, other._buf);
    mem::swap(_len, other._len);
  }

  static auto with_capacity(usize capacity) -> Vec {
    auto res = Vec{Buf<T>::with_capacity(capacity), 0};
    return res;
  }

  static auto from(auto&& v) -> Vec {
    auto res = Vec{};
    res.extend_from_slice(Slice<const T>{v});
    return res;
  }

  static auto from_iter(auto&& iter) -> Vec {
    auto res = Vec{};
    res.extend(iter);
    return res;
  }

  [[sfc_inline]] auto as_ptr() const -> const T* {
    return _buf._ptr;
  }

  [[sfc_inline]] auto as_mut_ptr() -> T* {
    return _buf._ptr;
  }

  [[sfc_inline]] auto len() const -> usize {
    return _len;
  }

  [[sfc_inline]] auto capacity() const -> usize {
    return _buf._cap;
  }

  [[sfc_inline]] auto is_empty() const -> bool {
    return _len == 0;
  }

  [[sfc_inline]] auto is_full() const -> bool {
    return _len == _buf._cap;
  }

  [[sfc_inline]] operator bool() const {
    return _len != 0;
  }

  [[sfc_inline]] auto as_slice() const -> slice::Slice<const T> {
    return {_buf._ptr, _len};
  }

  [[sfc_inline]] auto as_mut_slice() -> slice::Slice<T> {
    return {_buf._ptr, _len};
  }

  void set_len(usize new_len) {
    new_len = cmp::min(new_len, _buf._cap);
    _len = new_len;
  }

  auto clone() const -> Vec {
    auto res = Vec::with_capacity(_len);
    res.extend_from_slice(this->as_slice());
    return res;
  }

  void fmt(auto& f) const {
    f.debug_list().entries(this->iter());
  }

 public:
  [[sfc_inline]] auto get_unchecked(usize idx) const -> const T& {
    return _buf._ptr[idx];
  }

  [[sfc_inline]] auto get_unchecked_mut(usize idx) -> T& {
    return _buf._ptr[idx];
  }

  [[sfc_inline]] auto operator[](usize idx) const -> const T& {
    assert_fmt(idx < _len, "Vec::[]: idx(={}) out of range(={})", idx, _len);
    return _buf[idx];
  }

  [[sfc_inline]] auto operator[](usize idx) -> T& {
    assert_fmt(idx < _len, "Vec::[]: idx(={}) out of range(={})", idx, _len);
    return _buf[idx];
  }

  [[sfc_inline]] auto operator[](Range<> ids) const -> slice::Slice<const T> {
    return this->as_slice()[ids];
  }

  [[sfc_inline]] auto operator[](Range<> ids) -> slice::Slice<T> {
    return this->as_mut_slice()[ids];
  }

  [[sfc_inline]] auto first() const -> const T& {
    assert_fmt(_len != 0, "Vec::first: vec is empty");
    return _buf[0];
  }

  [[sfc_inline]] auto last() const -> const T& {
    assert_fmt(_len != 0, "Vec::last: vec is empty");
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

    ptr::write(_buf._ptr + _len, static_cast<T&&>(val));
    _len += 1;
  }

  auto pop() -> Option<T> {
    if (_len == 0) {
      return {};
    }

    auto ret = static_cast<T&&>(_buf[_len - 1]);
    this->truncate(_len - 1);
    return ret;
  }

  void truncate(usize len) {
    if (len >= _len) {
      return;
    }

    ptr::drop(_buf._ptr + len, _len - len);
    _len = len;
  }

  void reserve(usize additional) {
    _buf.reserve(_len, additional);
  }

  void reserve_extract(usize additional) {
    _buf.reserve_extract(_len, additional);
  }

  void shrink_to(usize min_capacity) {
    if (_buf._cap <= _len) {
      return;
    }
    _buf.shrink_to(_len, min_capacity);
  }

  void shrink_to_fit() {
    if (_buf._cap <= _len) {
      return;
    }
    _buf.shrink_to(_len, _len);
  }

  void clear() {
    this->truncate(0);
  }

  // swap with last, then remove
  void swap_remove(usize idx) {
    if (idx >= _len) {
      return;
    }

    if (idx != _len - 1) {
      _buf[idx] = static_cast<T&&>(_buf[_len - 1]);
    }

    this->truncate(_len - 1);
  }

  void insert(usize idx, T element) {
    idx = cmp::min(idx, _len);

    this->reserve(1);
    const auto p = _buf._ptr + idx;
    ptr::move(p, p + 1, _len - idx);
    _len += 1;

    _buf[idx] = static_cast<T&&>(element);
  }

  auto remove(usize idx) -> T {
    assert_fmt(idx < _len, "Vec::remove: idx(={}) out of range(={})", idx, _len);

    auto res = static_cast<T&&>(_buf[idx]);
    this->drain({idx, idx + 1});
    return res;
  }

  void drain(Range<> range) {
    auto tmp = (*this)[range];
    if (!tmp) {
      return;
    }

    // mov: [dst,src<<<end]
    const auto rem = _len - static_cast<usize>(tmp._ptr + tmp.len() - _buf._ptr);
    ptr::move(tmp._ptr + tmp._len, tmp._ptr, rem);
    this->truncate(_len - tmp.len());
  }

  void resize(usize new_len, T value) {
    if (new_len <= _len) {
      this->truncate(new_len);
      return;
    }

    const auto additional = new_len - _len;
    this->extend_with(additional, value);
  }

  void append(Vec& other) {
    this->reserve(other.len());

    ptr::uninit_move(other._buf._ptr, _buf._ptr + _len, other._len);
    this->set_len(_len + other._len);
  }

  void extend(auto&& iter) {
    this->reserve(iter.len());
    iter.for_each([&](auto&& x) { this->push(T{static_cast<decltype(x)&&>(x)}); });
  }

  template <class U, usize N>
  void extend(U (&v)[N]) {
    this->reserve(N);
    for (const auto& e : v) {
      this->push(e);
    }
  }

  void extend_with(usize n, T value) {
    this->reserve(n);
    for (auto i = 0U; i < n; ++i) {
      this->push(value);
    }
  }

  void extend_from_slice(Slice<const T> other) {
    if (other._len == 0) {
      return;
    }

    this->reserve(other._len);
    ptr::uninit_copy(other._ptr, _buf._ptr + _len, other._len);
    this->set_len(_len + other._len);
  }

  void retain(auto&& f) {
    auto new_len = 0UL;
    for (auto& x : this->as_mut_slice()) {
      if (!f(x)) {
        continue;
      }
      auto& y = _buf[new_len];
      if (&x != &y) {
        y = static_cast<T&&>(x);
      }
      new_len += 1;
    }
    this->truncate(new_len);
  }

 public:
  using Iter = typename slice::Iter<const T>;
  auto iter() const -> Iter {
    return this->as_slice().iter();
  }

  using IterMut = typename slice::Iter<T>;
  auto iter_mut() -> IterMut {
    return this->as_mut_slice().iter_mut();
  }

  auto windows(usize n) const {
    return this->as_slice().windows(n);
  }

  auto windows_mut(usize n) {
    return this->as_mut_slice().windows_mut(n);
  }

  auto truncks(usize n) const {
    return this->as_slice().truncks(n);
  }

  auto truncks_mut(usize n) {
    return this->as_mut_slice().truncks_mut(n);
  }

  auto contains(const auto& val) const -> bool {
    return this->as_slice().contains(val);
  }

  auto find_if(auto&& pred) const -> Option<usize> {
    return this->as_slice().find_if(pred);
  }

  auto rfind_if(auto&& pred) const -> Option<usize> {
    return this->as_slice().rfind_if(pred);
  }
};

}  // namespace sfc::vec

namespace sfc {
using vec::Vec;
}  // namespace sfc
