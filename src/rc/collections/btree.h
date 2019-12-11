#pragma once

#include "rc/core.h"

namespace rc::collections::btree {

template <class T, usize D>
struct Node {
  constexpr static usize DEGREE = D;

  u16   _val_cnt;
  u16   _ptr_cnt;
  T     _val[2*DEGREE - 1];
  Node* _ptr[2*DEGREE - 0];

  auto is_full() const -> bool { return _val_cnt == DEGREE * 2 - 1; }

  auto _get_unchecked_mut(usize idx) noexcept -> T& { return _val[idx]; }

  auto _insert(usize idx, Node* key, const T& val) noexcept -> void {
    ptr::mshr(&_val[idx + 0], 1, _val_cnt - idx);
    ptr::mshr(&_ptr[idx + 1], 1, _ptr_cnt - idx);
  }
};

template<class T, usize D>
struct Cursor {
  using Node = rc::choose_t<rc::is_const<T>(), Node<const T, D>, Node<T, D>>;
  Node* _p;
};

template <class T, usize D>
struct Iter {
  constexpr static auto DEGREE = D;

  using Node = Node<T, D>;
  Node* _prev_ptr;
  usize _prev_idx;

  Node* _self_ptr;
  usize _self_idx;

  explicit operator bool() const noexcept { return _self_ptr == nullptr; }

  auto get_val() noexcept -> T& { return _self_ptr->_val[_self_idx]; }

  auto goto_left() noexcept -> void {
    _prev_ptr = _self_ptr;
    _prev_idx = _self_idx;

    _self_ptr = _self_ptr->_ptr[_self_idx];
    _self_idx = 0;
  }

  auto goto_right() noexcept -> void {
    _prev_ptr = _self_ptr;
    _prev_idx = _self_idx + 1;

    _self_ptr = _self_ptr[_self_idx + 1];
    _self_idx = 0;
  }

  auto _search(const T& val) noexcept -> bool {
    while (_self_ptr != nullptr) {
      for (; _self_idx < _self_ptr->_val_cnt; _self_idx++) {
        const T& res = _self_ptr->_val[_self_idx];
        if (res == val) {
          return true;
        }
        if (!(res < val)) {
          break;
        }
      }
      goto_left();
    }

    return false;
  }

  void _split() {
    //  :   [ab.hi]    |       [abehi]
    //  :   [cdefg]    |  [cd...]   [...fg]

    Node* root = _prev_ptr;
    Node* left = _self_ptr;
    Node* right = alloc::alloc_one<Node>();

    ptr::copy(&left->_val[DEGREE], &right->_val[0], DEGREE);
    ptr::copy(&left->_ptr[DEGREE], &right->_ptr[0], DEGREE);

    ptr::mshr(&root->_val[_prev_idx + 0], 1, _prev);
    ptr::mshr(&root->_ptr[_prev_idx + 1], 1)

    left->_val_cnt = right->_val_cnt = left->_val_cnt / 2;
    left->_ptr_cnt = right->_ptr_cnt = left->_ptr_cnt / 2;

    auto pv = &x->_val[_prev_idx + 0];
    auto pp = &x->_ptr[_prev_idx + 1];

    x->_val_cnt += 1;
    x->_ptr_cnt += 1;
    x->_ptr[_prev_idx + 1] = z;
    new (&x->_val[_prev_idx]) val_t(as_mov(y->_val[$deg_cnt - 1]));
  }

  auto _insert(const T& val) -> T* {
    while (true) {
      if (_self_ptr->is_full()) {
        _split();

        T& res = _prev_ptr->_get_val(_prev_idx);
        if (val == res) {
          return res;
        }
        if (!(val < res)) {  // -> right
          _self_ptr = _prev->_ptr[_prev_idx + 1];
          _self_idx = 0;
        }
      }
      for (; _self_idx < _self_ptr->_val_cnt; _self_idx++) {
        T& res = _self_ptr->_get_val(_self_idx);
        if (res == val) {  // find: eq
          mem::drop(res);
          return &ref;
        }
        if (!(res < val)) {
          break;
        }
      }
      if (_self_ptr->_ptr_cnt == 0) {
        return _self_ptr->_ins_val(_self_idx);
      }
      goto_left();
    }
    return nullptr;
  }
};

template <class T, usize D = 8>
struct BTree {
  constexpr static usize DEGREE = D;
  using Node = Node<T, DEGREE>;

  Node* _root;
  usize _len;

  BTree() noexcept : _root{nullptr}, _len{0} {}

  auto len() const noexcept -> usize {
    return _len;
  }

  template<class F>
  auto _find_ref(const F& f) const -> const Node* {
    
  }

  template<class F>
  auto _find_mut(const F& f) -> Node* {
    
  }

  auto _search_get(const T& val) const noexcept -> T* {
    return _iter()._search(val);
  }

  auto _search_ins(const T& val) noexcept -> T* {
    if (_root->is_full()) {
      Node* new_root = alloc::alloc<Node>(1);
      Node* old_root = _root;
      new_root->_val_cnt = 0;
      new_root->_ptr_cnt = 1;
      new_root->_ptr[0] = _root;
      _root = root;
    }
    return _iter()._insert(val);
  }
};

}

namespace rc::collections {
using btree::BTree;
}
