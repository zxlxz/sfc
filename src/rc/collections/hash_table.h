#pragma once

#include "rc/alloc.h"

namespace rc::collections::hash_table {

template <class T>
struct Node {
  using Item = T;
  using Hash = typename Item::Hash;

  constexpr static auto HASH_NULL = num::MAX<Hash>;

  Node* _next = nullptr;
  Item _item = {};

  auto hash() const noexcept -> Hash { return _item.hash(); }

  auto set_hash(const Hash& val) noexcept -> void { _item.set_hash(val); }

  template <class K>
  auto eq(const K& key) const noexcept -> bool {
    return _item.eq(key);
  }
};

template <class T>
class Bucket {
 public:
  using Node = Node<T>;
  constexpr static auto HASH_NULL = Node::HASH_NULL;

  Node _head;

  Bucket() : _head() {}

  ~Bucket() {
    if (_head.hash() == HASH_NULL) {
      return;
    }
    mem::dtor(_head);

    for (Node* itr = _head._next; itr != nullptr;) {
      auto tmp = itr;
      itr = itr->_next;
      mem::dtor(tmp);
      mem::mdel(tmp);
    }
  }

  template <class U>
  auto search(const U& key) const -> Node* {
    if (_head.hash() == HASH_NULL) {
      return nullptr;
    }
    if (_head.eq(key)) {
      return as_mut(&_head);
    }
    for (Node* iter = _head._next; iter != nullptr; iter = iter->_next) {
      if (iter->eq(key)) {
        return iter;
      }
    }
    return nullptr;
  }

  auto insert(Node* node) -> void {
    node->_next = _head._next;
    _head._next = node;
  }

  auto erase(Node* node) -> void {
    if (_head.hash() == HASH_NULL) {
      return;
    }

    mem::drop(node->_data);

    if (node == &_head) {
      node->set_hash(HASH_NULL);

      if (_head._next != nullptr) {
        ptr::write(&_head, as_mov(_head->_next));
      }
    } else {
      for (Node* prev = &_head; prev != nullptr; prev = prev->_next) {
        if (prev->_next == node) {
          prev->_next = node->_next;
          return;
        }
      }
    }
  }
};

template <class T>
class HashTable {
 public:
  using Node = Node<T>;
  using Bucket = Bucket<T>;
  using Vec = vec::Vec<Bucket>;

  Vec _vec;
  usize _len;
  usize _mask;

  HashTable(usize capacity = 8) : _len(0), _mask(0) {
    auto cnt = num::next_power_of_tow(capacity);
    _mask = cnt - 1;
    _vec._extend(cnt, {Bucket()});
  }

  ~HashTable() {}

  auto len() const -> usize { return _len; }

  auto index(const K& key) const -> const T& {
    auto res = _search_get(key);
    if (res == nullptr) {
      throw Error::NotFound;
    }
    return res->_data;
  }

  template <class U>
  auto insert(U&& val) -> T& {
    auto res = _search_ins(val);
    ptr::write(&res->_data, as_fwd<U>(val));
    return res->_data;
  }

  template <class K>
  auto _search_get(const K& key) const -> const Node* {
    auto bin = key.hash() & _mask;
    auto& bkt = _vec[bin];
    auto res = bkt.search(key);
    return res;
  }

  template <class K>
  auto _search_ins(const K& key) -> Node* {
    if (_len >= _vec.len()) {
      _resize(_vec.len() * 2);
    }
    _len += 1;

    auto bin = key.hash() & _mask;
    Bucket& bkt = _vec[bin];

    // reset?
    if (bkt._head.hash() == HASH_NULL) {
      return &bkt._head;
    }

    auto res = bkt.search(key);

    // update?
    if (res != nullptr) {
      mem::dtor(*res);
    }
    // insert
    else {
      res = mem::mnew<Node>(1);
      bkt.insert(res);
    }
    return res;
  }

  auto _search_del(Node& val) -> Node& {
    auto bin = val._item.hash() & _mask;
    auto& bkt = _vec[bin];
    bkt.erase(val);
    _len -= 1;
  }

 private:
  auto _reserve(usize new_size) -> void {
    HashTable res(new_size);

    for (Bucket& bucket : _vec.iter_mut()) {
      Node* head = &bucket._head;
      if (head->_item.is_null()) {
        continue;
      }
      res.insert({ptr::read(&head->_item)});

      for (Node* node = head->_next; node != nullptr; node = node->_next) {
        res.insert({ptr::read(&node->_item)});
      }
    }
    _vec._len = 0;

    mem::swap(_vec, res);
  }
};

}  // namespace rc::collections::hash_table

namespace rc::collections {
using hash_table::HashTable;
}
