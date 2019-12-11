#pragma once

#include "rc/alloc.h"

namespace rc::collections::linked_list {

template <class T>
struct Node {
  using Item = T;
  Node* _prev;
  Node* _next;

  T _data;
  auto into_element() && -> T&& { return static_cast<T&&>(_data); }
};

template <class T>
struct Cursor {
  const Node<T>* _ptr;

  auto operator++() noexcept -> void { _ptr = _ptr->_next; }
  auto operator--() noexcept -> void { _ptr = _ptr->_prev; }
  auto operator*() const noexcept -> const T& { return _ptr->_data; }
  auto operator!=(const Cursor& other) const noexcept -> bool {
    return _ptr != other._ptr;
  }
};

template <class T>
struct CursorMut {
  Node<T>* _ptr;

  auto operator++() noexcept -> void { _ptr = _ptr->_next; }
  auto operator--() noexcept -> void { _ptr = _ptr->_prev; }
  auto operator*() const noexcept -> T& { return _ptr->_data; }
  auto operator!=(const CursorMut& other) const noexcept -> bool {
    return _ptr != other._ptr;
  }
};

template <class T>
struct Iter : iter::Iterator<Iter<T>> {
  const Node<T>* _head;
  const Node<T>* _tail;

  auto next() noexcept -> Option<const T&> {
    const auto res = _head;
    if (_head == nullptr) {
      _head = nullptr;
      _tail = nullptr;
    } else {
      _head = _head->_next;
    }
    return {res};
  }

  auto next_back() noexcept -> Option<const T&> {
    const auto res = _tail;
    if (_tail == nullptr) {
      _head = nullptr;
      _tail = nullptr;
    } else {
      _tail = _tail->_prev;
    }
    return {res};
  }
};

template <class T>
struct IterMut : iter::Iterator<Iter<T>> {
  Node<T>* _head;
  Node<T>* _tail;

  auto next() noexcept -> Option<T&> {
    const auto res = _head;
    if (_head == nullptr) {
      _head = nullptr;
      _tail = nullptr;
    } else {
      _head = _head->_next;
    }
    return {res};
  }

  auto next_back() noexcept -> Option<T&> {
    const auto res = _tail;
    if (_tail == nullptr) {
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

  using Cursor = linked_list::Cursor<T>;
  using CursorMut = linked_list::CursorMut<T>;
  using Iter = linked_list::Iter<T>;
  using IterMut = linked_list::IterMut<T>;

  Node* _head;
  Node* _tail;
  usize _len;

  LinkedList() noexcept : _head{nullptr}, _tail{nullptr}, _len{0} {}

  ~LinkedList() { this->clear(); }

  auto len() const noexcept -> usize { return _len; }

  auto is_empty() const -> bool { return _len == 0; }

  auto begin() const -> Cursor { return Cursor{_head}; }
  auto end() const -> Cursor { return Cursor{nullptr}; }

  auto begin() -> CursorMut { return CursorMut{_head}; }
  auto end() -> CursorMut { return CursorMut{nullptr}; }

  auto iter() -> Iter{ return {_head, _tail}; }
  auto iter_mut() -> IterMut { return {_head, _tail}; }

  auto clear() {
    for (Node* p = _head; p != nullptr;) {
      auto box = Box<Node>::from_raw(p);
      p = p->_next;
      (void)box;
    }
    _head = nullptr;
    _tail = nullptr;
  }

  auto push_front(T val) -> void {
    _push_front(Box<Node>{{nullptr, _head, static_cast<T&&>(val)}});
  }

  auto pop_front() -> T {
    rc::assert(_len != 0, u8"rc::collections::LinedList: empty");
    return (*_pop_front()).into_element();
  }

  auto push_back(T val) -> void {
    _push_back(Box<Node>{{_tail, nullptr, static_cast<T&&>(val)}});
  }

  auto pop_back() -> T {
    rc::assert(_len != 0, u8"rc::collections::LinedList: empty");
    return (*_pop_back()).into_element();
  }

  auto _push_front(Box<Node> node) noexcept -> void {
    Node* p = rc::move(node).into_raw();
    if (_head == nullptr) {
      _tail = p;
    } else {
      _head->_prev = p;
    }
    _head = p;
    _len = _len + 1;
  }

  auto _pop_front() noexcept -> Box<Node> {
    if (_len == 0) {
      return Box<Node>{nullptr};
    }
    const auto res = _head;
    _head = _head->_next;
    if (_head == nullptr) {
      _tail = nullptr;
    } else {
      _head->_prev = nullptr;
    }
    _len = _len - 1;
    return Box<Node>{res};
  }

  auto _push_back(Box<Node> node) noexcept -> void {
    const auto res = rc::move(node).into_raw();
    if (_tail == nullptr) {
      _head = res;
    } else {
      _tail->_next = res;
    }
    _tail = res;
    _len = _len + 1;
  }

  auto _pop_back() -> Box<Node> {
    if (_len == 0) {
      return Box<Node>{nullptr};
    }

    const auto res = _tail;
    _tail = _tail->_prev;
    if (_tail == nullptr) {
      _head = nullptr;
    } else {
      _tail->_next = nullptr;
    }
    _len = _len - 1;
    return Box<Node>{res};
  }

  template <class Out>
  auto fmt(fmt::Formatter<Out>& formatter) const {
    auto dbg = formatter.debug_list();
    for (auto& val : *this) {
      dbg.entry(val);
    }
  }
};

}  // namespace rc::collections::linked_list

namespace rc::collections {
using linked_list::LinkedList;
}
