#pragma once

#include "rc/alloc.h"

namespace rc::collections::binary_heap {

template <class T>
struct BinaryHeap {
  Vec<T> _vec;

  BinaryHeap() = default;
  ~BinaryHeap() = default;

  explicit BinaryHeap(Vec<T>&& v) noexcept : _vec{rc::move(v)} {}
  BinaryHeap(BinaryHeap&& other) noexcept = default;

  static BinaryHeap with_capacity(usize cap) noexcept {
    return {Vec<T>::with_capacity(cap)};
  }

  auto is_empty() const noexcept -> bool { return _vec.is_empty(); }

  auto len() const noexcept -> usize { return _vec.len(); }

  auto capacity() const noexcept -> usize { return _vec.capacity(); }

  auto top() const -> const T& {
    rc::assert(!this->is_empty(), u8"rc::collections::BinaryHeap: empty");
    return _vec[0];
  }

  auto push(T val) -> void {
    _vec.push(static_cast<T&&>(val));
    this->_shift_up(_vec.len() - 1);
  }

  auto pop() -> T {
    rc::assert(!this->is_empty(), u8"rc::collections::BinaryHeap: empty");
    auto res = _vec.swap_remove(0);
    _shift_down(0);
    return res;
  }

  auto _shift_up(usize idx_node) noexcept -> void {
    while (idx_node > 0) {
      const auto idx_root = (idx_node - 1) / 2;
      if (_vec[idx_root] <= _vec[idx_node]) {
        break;
      }
      mem::swap(_vec[idx_node], _vec[idx_root]);
      idx_node = idx_root;
    }
  }

  auto _shift_down(usize idx_root) noexcept -> void {
    const auto cnt = _vec.len();

    while (idx_root * 2 + 1 < cnt) {
      usize idx_left = idx_root * 2 + 1;
      usize idx_right = idx_root * 2 + 2;
      usize idx_next =
          idx_right < cnt
              ? (_vec[idx_left] <= _vec[idx_right] ? idx_left : idx_right)
              : idx_left;

      if (_vec[idx_root] <= _vec[idx_next]) {
        break;
      }
      mem::swap(_vec[idx_root], _vec[idx_next]);
      idx_root = idx_next;
    }
  }
};

}  // namespace rc::collections::binary_heap

namespace rc::collections {
using binary_heap::BinaryHeap;
}
