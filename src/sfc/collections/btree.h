#pragma once

#include "sfc/alloc.h"

namespace sfc::collections::btree {

template <class K, class V, usize B = 6>
struct BTree {
  struct Node;

  struct Entry {};

  Node* _root = nullptr;
  usize _len = 0;

 public:
  BTree() {}

  ~BTree() {
    this->clear();
  }

  BTree(BTree&& other) noexcept : _root{other._root} {
    other._root = nullptr;
  }

  auto len() const -> usize {
    return _len;
  }

  void clear();

  void append(BTree& other);

  auto entry(K key) -> Entry;

  auto get(const auto& key) const -> Option<const V&>;

  auto get_mut(const auto& key) -> Option<V&>;

  auto insert(K key, V value) -> Option<V>;

  auto remove(const auto& key) -> Option<V>;
};

template <class K, class V, usize B>
struct BTree<K, V, B>::Node {
  static constexpr usize CAPACITY = 2 * B - 1;

  Node* _parent = nullptr;
  u16 _idx = 0;
  u16 _len = 0;

  mem::Uninit<K> _keys[CAPACITY];
  mem::Uninit<V> _vals[CAPACITY];
  Node* _edge[CAPACITY + 1];

 public:
 private:
  auto search_for_insert(auto& key) -> Tuple<> {

  }

  auto split() -> Node* {
    if (_parent == nullptr) {
      _parent = new Node{};
      _parent->_edge[0] = this;
    }

    auto lhs = this;
    auto rhs = new Node{};
    lhs._len = rhs._len = B - 1;
    ptr::move(_keys, rhs._keys);
    ptr::move(_vals, rhs._vals);
    ptr::move(_edge, rhs._edge);
    _parent->insert_fit(_idx + 1, mem::move(*&_keys[B - 1]), mem::move(*&_vals[B - 1]), rhs);

    ptr::drop(_keys + B - 1, B);
    ptr::drop(_vals + B - 1, B);
  }

  void insert_fit(usize idx, K key, V val, Node* node = nullptr) {
    ptr::move(_keys + idx, _keys + idx + 1, _len - idx);
    ptr::move(_vals + idx, _vals + idx + 1, _len - idx);
    _keys[idx] = static_cast<K&&>(key);
    _vals[idx] = static_cast<V&&>(val);

    if (node != nullptr) {
      ptr::move(_edge + idx + 1, _edge + idx + 2, _len - idx);
      _edge[idx + 1] = node;
    }
  }
};

}  // namespace sfc::collections::btree
