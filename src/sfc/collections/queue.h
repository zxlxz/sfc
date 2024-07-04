#pragma once

#include "sfc/alloc.h"

namespace sfc::collections {

template <class T>
class [[nodiscard]] Queue {
 protected:
  Vec<T> _vec;
  usize _pos = 0;

  Queue(Vec<T>&& vec, usize pos) noexcept : _vec{static_cast<Vec<T>&&>(vec)}, _pos{pos} {}

 public:
  Queue() = default;

  ~Queue() = default;

  Queue(Queue&& other) noexcept : _vec{mem::move(other._vec)}, _pos{mem::take(other._pos)} {}

  Queue& operator=(Queue&& other) noexcept {
    _vec = mem::move(other._vec);
    _pos = mem::take(other._pos);
    return *this;
  }

  static auto with_capacity(usize capacity) -> Queue {
    return Queue{Vec<T>::with_capacity(capacity), 0};
  }

  auto capacity() const -> usize {
    return _vec.capacity();
  }

  auto len() const -> usize {
    return _vec.len() - _pos;
  }

  auto is_empty() const -> bool {
    return this->len() == 0;
  }

  explicit operator bool() const {
    return this->len() != 0;
  }

  auto as_slice() const -> slice::Slice<const T> {
    return _vec[{_pos, _}];
  }

  auto as_mut_slice() -> slice::Slice<T> {
    return _vec[{_pos, _}];
  }

  auto clone() const -> Queue {
    auto ret = Queue::with_capacity(this->len());
    ret.extend(*this);
    return ret;
  }

 public:
  auto operator[](usize idx) const -> const T& {
    const auto len = this->len();
    assert_fmt(idx < len, "Queue::[]: idx(={}) out of range(={})", idx, len);

    return _vec[_pos + idx];
  }

  auto operator[](usize idx) -> T& {
    const auto len = this->len();
    assert_fmt(idx < len, "Queue::[]: idx(={}) out of range(={})", idx, len);

    return _vec[_pos + idx];
  }

  auto first() const -> const T& {
    assert_fmt(!this->is_empty(), "Queue::first: queue is empty");
    return _vec[_pos];
  }

  auto first_mut() -> T& {
    assert_fmt(!this->is_empty(), "Queue::first_mut: queue is empty");
    return _vec[_pos];
  }

  auto last() const -> const T& {
    assert_fmt(!this->is_empty(), "Queue::last: queue is empty");
    return _vec[_vec.len() - 1];
  }

  auto last_mut() -> T& {
    assert_fmt(!this->is_empty(), "Queue::last_mut: queue is empty");
    return _vec[_vec.len() - 1];
  }

 public:
  void push(T element) {
    this->reserve(1);
    _vec.push(static_cast<T&&>(element));
  }

  auto pop() -> Option<T> {
    if (_pos == _vec.len()) {
      return {};
    }

    auto res = static_cast<T&&>(_vec[_pos]);
    _pos += 1;
    return res;
  }

  void insert(usize idx, T element) {
    this->reserve(1);
    _vec.insert(_pos + idx, static_cast<T&&>(element));
  }

  void reserve(usize additional) {
    const auto cur_len = _vec.len();
    const auto old_cap = _vec.capacity();
    if (cur_len + additional <= old_cap) {
      return;
    }

    const auto vec_rem = _pos;
    if (vec_rem > old_cap / 2) {
      this->shrink_to_fit();
    }

    _vec.reserve(additional);
  }

  void remove(usize idx) {
    _vec.remove(_pos + idx);
  }

  void drain(Range<> range) {
    range = range % this->len();
    _vec.drain({range._start + _pos, range._end + _pos});
  }

  void truncate(usize len) {
    if (len >= this->len()) {
      return;
    }
    const auto cnt = this->len() - len;
    _vec.drain({_pos, _pos + cnt});
  }

  void clear() {
    _pos = 0;
    _vec.clear();
  }

  void shrink_to_fit() {
    if (_pos == 0) {
      return;
    }
    _vec.drain({0, _pos});
    _pos = 0;
  }

  void extend(auto&& iter) {
    for (T ele : iter) {
      this->push(static_cast<T&&>(ele));
    }
  }

  void retain(auto&& f) {
    auto new_len = 0UL;
    for (auto& x : this->as_mut_slice()) {
      if (!f(x)) {
        continue;
      }
      auto& y = _vec.get_unchecked_mut(new_len);
      if (&x != &y) {
        y = static_cast<T&&>(x);
      }
      new_len += 1;
    }
    _vec.truncate(new_len);
    _pos = 0;
  }

 public:
  using Iter = slice::Iter<const T>;
  auto iter() const -> Iter {
    return {_vec.as_ptr() + _pos, this->len()};
  }

  using IterMut = slice::Iter<T>;
  auto iter_mut() -> IterMut {
    return {_vec.as_mut_ptr() + _pos, this->len()};
  }

  void fmt(auto& f) const {
    this->as_slice().fmt(f);
  }
};

}  // namespace sfc::collections

namespace sfc {
using collections::Queue;
}  // namespace sfc
