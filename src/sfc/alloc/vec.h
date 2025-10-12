#pragma once

#include "sfc/alloc/alloc.h"

namespace sfc::vec {

template <class T, class A = alloc::Global>
class [[nodiscard]] Buf {
 public:
  T* _ptr{nullptr};
  usize _cap{0};
  [[no_unique_address]] A _alloc{};

 public:
  Buf() noexcept {}

  ~Buf() noexcept {
    if (_ptr) {
      _alloc.dealloc(_ptr, alloc::Layout::array<T>(_cap));
    }
  }

  Buf(Buf&& other) noexcept : _ptr{other._ptr}, _cap{other._cap}, _alloc{other._alloc} {
    other._ptr = {};
    other._cap = {};
    other._alloc = {};
  }

  auto operator=(Buf&& other) noexcept -> Buf& {
    if (this == &other) {
      return *this;
    }

    if (_ptr) {
      _alloc.dealloc(_ptr, alloc::Layout::array<T>(_cap));
    }

    _ptr = mem::take(other._ptr);
    _cap = mem::take(other._cap);
    _alloc = mem::move(other._alloc);
    return *this;
  }

  static auto with_capacity(usize capacity) -> Buf {
    auto res = Buf{};
    res.reserve(0, capacity);
    return res;
  }

  auto ptr() const noexcept -> T* {
    return _ptr;
  }

  auto capacity() const noexcept -> usize {
    return _cap;
  }

  auto operator[](usize idx) const noexcept -> const T& {
    return _ptr[idx];
  }

  auto operator[](usize idx) noexcept -> T& {
    return _ptr[idx];
  }

  void reserve(usize used, usize additional) {
    const auto req_cap = used + additional;
    if (req_cap <= _cap) {
      return;
    }

    const auto min_cap = _cap < 8U ? 8U : _cap * 2;
    const auto new_cap = req_cap < min_cap ? min_cap : req_cap;
    this->realloc(used, new_cap);
  }

  void reserve_extract(usize used, usize additional) {
    const auto new_cap = used + additional;
    if (new_cap == _cap) {
      return;
    }

    this->realloc(used, new_cap);
  }

  void realloc(usize used, usize new_cap) {
    if (new_cap == _cap) {
      return;
    }

    const auto new_ptr = static_cast<T*>(_alloc.alloc(alloc::Layout::array<T>(new_cap)));
    if (_ptr) {
      ptr::uninit_move(_ptr, new_ptr, used);
      _alloc.dealloc(_ptr, alloc::Layout::array<T>(_cap));
    }

    _ptr = new_ptr;
    _cap = new_cap;
  }
};

template <class T, class A = alloc::Global>
class [[nodiscard]] Vec {
  using Buf = vec::Buf<T, A>;

  Buf _buf = {};
  usize _len = 0;

 public:
  Vec() noexcept = default;

  ~Vec() noexcept {
    this->clear();
  }

  Vec(Vec&& other) noexcept : _buf{static_cast<Buf&&>(other._buf)}, _len{other._len} {
    other._len = 0;
  }

  Vec& operator=(Vec&& other) noexcept {
    if (this == &other) {
      return *this;
    }
    this->clear();
    _buf = mem::move(other._buf);
    _len = mem::take(other._len);
    return *this;
  }

  static auto with_capacity(usize capacity) -> Vec {
    auto res = Vec{};
    res.reserve(capacity);
    return res;
  }

  static auto from(Slice<const T> v) -> Vec {
    auto res = Vec{};
    res.extend_from_slice(v);
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

  explicit operator bool() const {
    return _len != 0;
  }

  auto as_slice() const -> slice::Slice<const T> {
    return {_buf._ptr, _len};
  }

  auto as_mut_slice() -> slice::Slice<T> {
    return {_buf._ptr, _len};
  }

  void set_len(usize new_len) noexcept {
    if (new_len <= _buf._cap) {
      _len = new_len;
    }
  }

  auto clone() const -> Vec {
    auto res = Vec{};
    res.extend_from_slice({_buf._ptr, _len});
    return res;
  }

 public:
  auto get_unchecked(usize idx) const -> const T& {
    return _buf._ptr[idx];
  }

  auto get_unchecked_mut(usize idx) -> T& {
    return _buf._ptr[idx];
  }

  auto operator[](usize idx) const -> const T& {
    panicking::expect(idx < _len, "Vec::[]: idx(={}) out of ids(={})", idx, _len);
    return _buf._ptr[idx];
  }

  auto operator[](usize idx) -> T& {
    panicking::expect(idx < _len, "Vec::[]: idx(={}) out of ids(={})", idx, _len);
    return _buf._ptr[idx];
  }

  auto operator[](ops::Range ids) const -> slice::Slice<const T> {
    return slice::Slice<const T>{_buf._ptr, _len}[ids];
  }

  auto operator[](ops::Range ids) -> slice::Slice<T> {
    return slice::Slice<T>{_buf._ptr, _len}[ids];
  }

  auto spare_capacity_mut() -> slice::Slice<T> {
    return slice::Slice{_buf._ptr + _len, _buf._cap - _len};
  }

  auto first() const -> option::Option<const T&> {
    if (_len == 0) {
      return {};
    }
    return _buf._ptr[0];
  }

  auto first_mut() -> option::Option<T&> {
    if (_len == 0) {
      return {};
    }
    return _buf._ptr[0];
  }

  auto last() const -> option::Option<const T&> {
    if (_len == 0) {
      return {};
    }
    return _buf._ptr[_len - 1];
  }

  auto last_mut() -> option::Option<T&> {
    if (_len == 0) {
      return {};
    }
    return _buf._ptr[_len - 1];
  }

  void fill(const T& val) {
    Slice{_buf._ptr, _len}.fill(val);
  }

  void fill_with(auto&& f) {
    Slice{_buf._ptr, _len}.fill_with(f);
  }

 public:
  auto push(T val) -> T& {
    this->reserve(1);

    auto dst = _buf._ptr + _len;
    new (dst) T{static_cast<T&&>(val)};
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

  auto swap_remove(usize idx) -> T {
    panicking::expect(idx < _len, "Vec::swap_remove: idx({}) out of ids([0,{}))", idx, _len);

    auto res = mem::move(_buf._ptr[idx]);
    if (idx != _len - 1) {
      _buf._ptr[idx] = mem::move(_buf._ptr[_len - 1]);
    }
    mem::drop(_buf._ptr[_len - 1]);
    _len -= 1;

    return res;
  }

  void truncate(usize len) {
    if (len >= _len) {
      return;
    }

    ptr::drop_in_place(_buf._ptr + len, _len - len);
    _len = len;
  }

  void reserve(usize additional) {
    if (_len + additional <= _buf._cap) {
      return;
    }
    _buf.reserve(_len, additional);
  }

  void reserve_extract(usize additional) {
    if (_len + additional <= _buf._cap) {
      return;
    }
    _buf.reserve_extract(_len, additional);
  }

  void shrink_to(usize min_cap) {
    if (min_cap < _len || min_cap >= _buf._cap) {
      return;
    }
    _buf.realloc(_len, min_cap);
  }

  void shrink_to_fit() {
    if (_len == _buf._cap) {
      return;
    }
    _buf.realloc(_len, _len);
  }

  void clear() {
    ptr::drop_in_place(_buf._ptr, _len);
    _len = 0;
  }

  void insert(usize idx, T val) {
    panicking::expect(idx <= _len, "Vec::insert: idx({}) out of ids([0,{}))", idx, _len);

    this->reserve(1);
    ptr::push_front(_buf._ptr + idx, _len - idx, static_cast<T&&>(val));
    _len += 1;
  }

  auto remove(usize idx) -> T {
    panicking::expect(idx < _len, "Vec::remove: idx({}) out of ids([0,{}))", idx, _len);

    auto res = ptr::pop_front(_buf._ptr + idx, _len - idx);
    _len -= 1;
    return res;
  }

  void drain(ops::Range ids) {
    auto tmp = (*this)[ids];
    if (tmp._len == 0) {
      return;
    }
    const auto tail_len = _len - static_cast<usize>(tmp._ptr + tmp._len - _buf._ptr);
    ptr::shift_elements_left(tmp._ptr + tmp._len, tail_len, tmp._len);
    _len -= tmp._len;
  }

  void resize(usize new_len, T value) {
    if (new_len <= _len) {
      return this->truncate(new_len);
    }

    this->extend_with(new_len - _len, value);
  }

  void append(Vec& other) {
    if (other._len == 0) {
      return;
    }

    this->reserve(other._len);
    ptr::uninit_move(other._buf._ptr, _buf._ptr + _len, other._len);
    _len += other._len;
    other._len = 0;
  }

  void extend(auto iter) {
    if constexpr (requires { iter.len(); }) {
      this->reserve(iter.len());
    }

    iter.for_each([&](T val) { this->push(static_cast<T&&>(val)); });
  }

  void extend_with(usize cnt, T value) {
    this->reserve(cnt);

    for (auto idx = 0UL; idx < cnt; ++idx) {
      this->push(static_cast<T&&>(value));
    }
  }

  void extend_from_slice(Slice<const T> other) {
    this->reserve(other._len);
    ptr::uninit_copy(other._ptr, _buf._ptr + _len, other._len);
    _len += other._len;
  }

  void retain(auto&& f) {
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
  auto iter() const -> slice::Iter<const T> {
    return Slice{_buf._ptr, _len}.iter();
  }

  auto iter_mut() -> slice::Iter<T> {
    return Slice{_buf._ptr, _len}.iter_mut();
  }

  auto windows(usize n) const -> slice::Windows<const T> {
    return Slice{_buf._ptr, _len}.windows(n);
  }

  auto windows_mut(usize n) -> slice::Windows<T> {
    return Slice{_buf._ptr, _len}.windows_mut(n);
  }

  auto chunks(usize n) const -> slice::Chunks<const T> {
    return Slice{_buf._ptr, _len}.chunks(n);
  }

  auto chunks_mut(usize n) -> slice::Chunks<T> {
    return Slice{_buf._ptr, _len}.chunks_mut(n);
  }

 public:
  // trait: fmt::Display
  void fmt(auto& f) const {
    return Slice{_buf._ptr, _len}.fmt(f);
  }

  // trait: serde::Serialize
  auto serialize(auto& ser) const {
    const auto v = Slice{_buf._ptr, _len};
    return v.serialize(ser);
  }

  // trait:: serde::Deserialize
  auto deserialize(auto& des) {
    this->clear();

    auto seq = des.deserialize_seq();
    while (true) {
      auto opt = _TRY(seq.next_element(T{}));
      if (!opt) {
        break;
      }
      this->push(mem::move(opt).unwrap());
    }
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
