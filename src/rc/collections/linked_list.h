#pragma once

#include "rc/alloc.h"

namespace rc::collections::linked_list {

template <class T>
struct Node {
  Node* _prev;
  Node* _next;
  T _data;
};

template <class T>
struct Cursor {
  Node<T>* _ptr;

  auto operator++() noexcept -> void { _ptr = _ptr->_next; }

  auto operator--() noexcept -> void { _ptr = _ptr->_prev; }

  auto operator*() const noexcept -> T& { return *_ptr; }

  auto operator!=(const Cursor& other) const noexcept -> bool {
    return _ptr != other._ptr;
  }
};

template <class T>
struct Iter : iter::Iterator<Iter<T>> {
  using Item = Node<T>&;

  Node<T>* _head;
  Node<T>* _tail;

  auto next() noexcept -> Option<T&> {
    const auto res = _head;
    if (_head == _tail) {
      _head = nullptr;
      _tail = nullptr;
    } else {
      _head = _head->_next;
    }
    return {res};
  }

  auto next_back() noexcept -> Option<T&> {
    const auto res = _head;

    if (_head == _tail) {
      _head = nullptr;
      _tail = nullptr;
    } else {
      _tail = _tail->_prev;
    }
    return {res};
  }
};

template <class T>
struct LinkedList {
  using Node = Node<T>;
  using Iter = linked_list::Iter<T>;
  using IterMut = linked_list::Iter<const T>;

  Node* _head;
  Node* _tail;

  LinkedList() : _head{nullptr}, _tail{nullptr} {}

  ~LinkedList() {
    Node* p = _head;

    while (p != nullptr) {
      const auto q = p;
      p = p->_next;
      alloc::dealloc(p, 1);
    }
  }

  bool is_empty() const->bool { return _head = nullptr; }

  auto iter_mut() -> IterMut { return {_head, _tail}; }

  auto clear() {}

  auto push_front(T val) -> void {
    Node* node = alloc::alloc<Node>(1);
    ptr::write(&node->_data, static_cast<T&&>(val));
    this->_push_front(node);
  }

  auto pop_front() noexcept -> Node {
    Node* node = this->_pop_front();
    rc::assert(node != nullptr, u8"rc::collections::LinkedList: empty");

    auto data = ptr::read(&node->_data);
    alloc::dealloc(node, 1);
    return data;
  }

  auto push_back(T val) -> void {
    Node* node = alloc::alloc<Node>(1);
    ptr::write(&node->_data, static_cast<T&&>(val));
    this->_push_back(node);
  }

  auto pop_back() noexcept -> Node {
    Node* node = this->_pop_back();
    rc::assert(node != nullptr, u8"rc::collections::LinkedList: empty");

    auto data = ptr::read(&node->_data);
    alloc::dealloc(node, 1);
    return data;
  }

  auto _push_front(Node* node) noexcept -> void {
    node->_next = _head;
    node->_prev = nullptr;

    if (_head == nullptr) {
      _tail = node;
    } else {
      _head->_prev = node;
    }
    _head = node;
  }

  auto _push_back(Node* node) noexcept -> void {
    node->_next = nullptr;
    node->_prev = _tail;

    if (_tail == nullptr) {
      _head = node;
    } else {
      _tail->_next = node;
    }
    _tail = node;
  }

  auto _pop_front() -> Node* {
    const auto node = _head;

    _head = _head->_next;
    if (_head == nullptr) {
      _tail = nullptr;
    } else {
      _head->_prev = nullptr;
    }
    return node;
  }

  auto _pop_back() -> Node* {
    if (_tail == nullptr) {
      return nullptr;
    }

    const auto node = _tail;
    _tail = _tail->_prev;
    if (_tail == nullptr) {
      _head = nullptr;
    } else {
      _tail->_next = nullptr;
    }
    return node;
  }
};

}  // namespace rc::collections::linked_list

namespace rc::collections {
using linked_list::LinkedList;
}
