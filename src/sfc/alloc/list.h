#pragma once

#include "sfc/alloc/raw_buf.h"

namespace sfc::list {

using slice::Range;

template <class T, class A = alloc::Global>
class [[nodiscard]] List {
  RawBuf<T, A> _buf{};
  usize _len{0};

  List(RawBuf<T, A> buf, usize len) noexcept : _buf{mem::move(buf)}, _len{len} {}

 public:
  List() noexcept = default;

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

  static auto new_(A alloc = {}) noexcept -> List {
    auto res = List{RawBuf<T, A>::new_(alloc), 0};
    return res;
  }

  static auto with_capacity(usize capacity, A alloc = {}) noexcept -> List {
    auto res = List{RawBuf<T, A>::with_capacity(capacity, alloc), 0};
    return res;
  }

  static auto from(Slice<const T> s, A alloc = {}) -> List {
    auto res = List::with_capacity(s.len(), alloc);
    res.extend_from_slice(s);
    return res;
  }

 public:
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

  auto operator*() const noexcept -> Slice<const T> {
    return {_buf.ptr(), _len};
  }

  auto operator*() noexcept -> Slice<T> {
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
    if (_len == 0) return {};

    _len -= 1;
    return ptr::read(&_buf[_len]);
  }

  auto swap_remove(usize idx) -> T {
    sfc::assert_(idx < _len, "List::swap_remove: idx({}) out of range([0, {}))", idx, _len);

    auto hole = _buf.ptr() + idx;
    auto value = ptr::read(hole);
    if (idx != _len - 1) {
      auto last = _buf.ptr() + (_len - 1);
      ptr::write(hole, ptr::read(last));
    }
    this->set_len(_len - 1);
    return value;
  }

  void truncate(usize len) {
    if (len >= _len) return;

    const auto remaining_len = _len - len;
    this->set_len(len);
    ptr::drop_in_place(_buf.ptr() + len, remaining_len);
  }

  void reserve(usize additional) {
    if (additional == 0) return;
    _buf.reserve(_len, additional);
  }

  void reserve_exact(usize additional) {
    if (additional == 0) return;
    _buf.reserve_exact(_len, additional);
  }

  void shrink_to(usize min_cap) {
    _buf.shrink_to_fit(min_cap);
  }

  void shrink_to_fit() {
    _buf.shrink_to_fit(_len);
  }

  void clear() noexcept {
    const auto elms = Slice{_buf.ptr(), _len};
    this->set_len(0);
    ptr::drop_in_place(elms._ptr, elms._len);
  }

  auto insert(usize idx, T val) -> T& {
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

  auto remove(usize idx) -> T {
    sfc::assert_(idx < _len, "List::remove: idx(={}) out of ids([0,{}))", idx, _len);

    const auto p = _buf.ptr() + idx;
    auto ret = ptr::read(p);
    ptr::copy(p + 1, p, _len - idx - 1);
    this->set_len(_len - 1);
    return ret;
  }

  void resize(usize new_len, T value) {
    if (new_len <= _len) {
      return this->truncate(new_len);
    }
    this->extend_with(new_len - _len, value);
  }

  void append(List& other) {
    sfc::assert_(&other != this, "List::append: cannot append self");
    this->extend_from_slice(other.as_slice());
    other.set_len(0);
  }

  void extend(auto&& iter) {
    if constexpr (requires { iter.len(); }) {
      this->reserve(iter.len());
    }
    iter.for_each([&](T val) { this->push(mem::move(val)); });
  }

  void extend_with(usize cnt, const T& value) {
    static_assert(trait::tv_copy_<T>, "List::extend_with: T must be trivially copyable");
    this->reserve(cnt);

    const auto p = _buf.ptr() + _len;
    for (auto idx = 0UL; idx < cnt; ++idx) {
      ptr::write(p + idx, value);
    }
    this->set_len(_len + cnt);
  }

  void extend_from_slice(Slice<const T> other) noexcept {
    static_assert(trait::tv_copy_<T>);
    this->reserve(other._len);
    ptr::copy_nonoverlapping(other._ptr, _buf.ptr() + _len, other._len);
    this->set_len(_len + other._len);
  }

  void retain(auto&& f) noexcept {
    const auto p = _buf.ptr();
    auto read = usize{0};
    for (; read < _len; read += 1) {
      if (!f(p[read])) {
        break;
      }
    }

    // all elems are kept, return early
    if (read == _len) {
      return;
    }

    auto g_read = read + 1;  // guard read
    auto g_write = read;     // guard write
    ptr::drop_in_place(p + read);
    // invariant: g_write < g_read (gap starts at 1 and never shrinks),
    // so the kept element never overlaps the hole it is copied into
    for (; g_read < _len; g_read += 1) {
      auto& cur = p[g_read];
      if (!f(cur)) {
        ptr::drop_in_place(&cur, 1);
      } else {
        const auto hole = p + g_write;
        ptr::copy_nonoverlapping(&cur, hole, 1);
        g_write += 1;
      }
    }
    this->set_len(g_write);
  }

  /// remove elmts in the specified range
  /// @note not same as Rust's drain
  void drain(Range range) noexcept {
    const auto [start, end] = range.wrap(_len);
    const auto p = _buf.ptr();
    if (start != end) {
      ptr::drop_in_place(p + start, end - start);
      ptr::copy(p + end, p + start, _len - end);
      this->set_len(_len - (end - start));
    }
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
    static_assert(trait::tv_copy_<T>);
    auto res = List::with_capacity(_len, _buf.allocator());
    res.extend_from_slice({_buf.ptr(), _len});
    return res;
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    return Slice{_buf.ptr(), _len}.fmt(f);
  }

  // trait: io::Write
  auto write(Slice<const T> buf) -> io::Result<usize> {
    static_assert(trait::tv_copy_<T>);
    this->extend_from_slice(buf);
    return buf.len();
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
  template <class D, class E = typename D::Error>
  static auto deserialize(D& des) -> Result<List, E> {
    auto res = List::new_();
    _TRY(des.as_seq().template for_each<T>([&](auto val) {
      res.push(mem::move(val));
    }));
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

}  // namespace sfc::list

namespace sfc {
using list::List;
}  // namespace sfc
