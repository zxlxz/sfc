#pragma once

#include "sfc/core.h"

namespace sfc::collections::btree {

template <class K, class V, usize B = 6>
struct BTree {
  static_assert(B >= 2, "B-tree branching factor must be at least 2");

  struct Node;
  struct Entry {};

 private:
  Node* _root = nullptr;
  usize _len = 0;

 public:
  BTree() = default;

  ~BTree() {
    clear();
  }

  BTree(BTree&& other) noexcept : _root{mem::take(other._root)}, _len{mem::take(other._len)} {}

  auto operator=(BTree&& other) noexcept -> BTree& {
    if (this != &other) {
      this->clear();
      _root = mem::take(other._root);
      _len = mem::take(other._len);
    }
    return *this;
  }

  auto len() const -> usize {
    return _len;
  }

  auto is_empty() const -> bool {
    return _len == 0;
  }

  void clear() {
    if (_root != nullptr) {
      _root->clear_recursive();
      delete _root;
      _root = nullptr;
      _len = 0;
    }
  }

  auto get(const K& key) const -> Option<const V&> {
    if (_root == nullptr) {
      return Option<const V&>{};
    }
    return _root->search(key);
  }

  auto get_mut(const K& key) -> Option<V&> {
    if (_root == nullptr) {
      return Option<V&>{};
    }
    return _root->search_mut(key);
  }

  auto insert(K key, V value) -> Option<V> {
    if (_root == nullptr) {
      _root = new Node{true};
    }

    auto old_value = _root->insert(mem::move(key), mem::move(value));

    // Handle root split
    if (_root->_len == 2 * B) {
      auto new_root = new Node{false};
      auto right = _root->split();
      new_root->_keys[0] = mem::move(_root->_keys[B - 1]);
      new_root->_vals[0] = mem::move(_root->_vals[B - 1]);
      new_root->_children[0] = _root;
      new_root->_children[1] = right;
      new_root->_len = 1;
      _root->_len = B - 1;
      _root = new_root;
    }

    if (!old_value.has_value()) {
      ++_len;
    }
    return old_value;
  }

  auto remove(const K& key) -> Option<V> {
    if (_root == nullptr) {
      return Option<V>{};
    }

    auto result = _root->remove(key);
    if (result.has_value()) {
      --_len;

      // Handle root underflow
      if (_root->_len == 0 && !_root->_is_leaf) {
        auto old_root = _root;
        _root = _root->_children[0];
        delete old_root;
      }
    }
    return result;
  }

  auto contains_key(const K& key) const -> bool {
    return get(key).has_value();
  }

  void append(BTree& other) {
    // Simple implementation - insert each element from other
    if (other._root != nullptr) {
      other._root->append_to(*this);
      other.clear();
    }
  }

  auto entry(K key) -> Entry {
    return Entry{};  // Placeholder
  }
};

template <class K, class V, usize B>
struct BTree<K, V, B>::Node {
  static constexpr usize MAX_KEYS = 2 * B - 1;
  static constexpr usize MIN_KEYS = B - 1;

  Node* _parent = nullptr;
  Node* _childs[MAX_KEYS + 1];
  u16 _idx = 0;
  u16 _len = 0;
  K _keys[MAX_KEYS];
  V _vals[MAX_KEYS];

 public:
  Node() = default;

  ~Node() = default;

  void clear_recursive() {
    for (auto i = 0U; i <= _len; ++i) {
      if (_childs[i] != nullptr) {
        _childs[i]->clear_recursive();
        delete _childs[i];
      }
    }
  }

  auto find(const K& key) const -> Option<const V&> {
    auto idx = 0U;
    for (; idx < _len; ++idx) {
      if (key == _keys[idx]) {
        return _vals[idx];
      }
      if (key < _keys[idx]) {
        break;
      }
    }
    if (idx == 0 || _children[idx] == nullptr) {
      return {};
    }

    auto next = _childs[idx];
    return next->find(key);
  }

  auto insert(K key, V val) -> V& {
    auto idx = 0U;
    for (; idx < _len; ++idx) {
      if (key == _keys[idx]) {
        _vals[idx] = static_cast<V&&>(val);
        return _vals[idx];
      }
      if (key < _keys[idx]) {
        break;
      }
    }

    if (_len == 0) {

    }
    
    if (_len != 0) {
      return _childs[idx]->insert(static_cast<K&&>(key), static_cast<V&&>(val));
    }

  }
  auto remove(const K& key) -> Option<V> {
    auto idx = find_key_index(key);

    if (idx < _len && _keys[idx] == key) {
      if (_is_leaf) {
        return remove_from_leaf(idx);
      } else {
        return remove_from_internal(idx);
      }
    }

    if (_is_leaf) {
      return Option<V>{};
    }

    auto result = _children[idx]->remove(key);

    // Handle child underflow
    if (_children[idx]->_len < MIN_KEYS) {
      fix_child_underflow(idx);
    }

    return result;
  }

  void append_to(BTree& tree) {
    // In-order traversal to append all elements
    for (usize i = 0; i < _len; ++i) {
      if (!_is_leaf) {
        _children[i]->append_to(tree);
      }
      tree.insert(mem::move(_keys[i]), mem::move(_vals[i]));
    }
    if (!_is_leaf && _len > 0) {
      _children[_len]->append_to(tree);
    }
  }

 private:
  void insert_at_index(usize idx, K key, V value) {
    // Shift elements to the right
    for (usize i = _len; i > idx; --i) {
      _keys[i] = mem::move(_keys[i - 1]);
      _vals[i] = mem::move(_vals[i - 1]);
    }

    _keys[idx] = mem::move(key);
    _vals[idx] = mem::move(value);
    ++_len;
  }

  auto split() -> Node* {
    auto right = new Node{_is_leaf};
    auto mid = B - 1;

    // Move keys and values to right node
    for (usize i = 0; i < B; ++i) {
      right->_keys[i] = mem::move(_keys[mid + 1 + i]);
      right->_vals[i] = mem::move(_vals[mid + 1 + i]);
    }

    // Move children if internal node
    if (!_is_leaf) {
      for (usize i = 0; i <= B; ++i) {
        right->_children[i] = _children[mid + 1 + i];
      }
    }

    right->_len = B;
    _len = mid;

    return right;
  }

  void split_child(usize idx) {
    auto child = _children[idx];
    auto new_child = child->split();

    // Move the middle key up
    for (usize i = _len; i > idx; --i) {
      _keys[i] = mem::move(_keys[i - 1]);
      _vals[i] = mem::move(_vals[i - 1]);
      _children[i + 1] = _children[i];
    }

    _keys[idx] = mem::move(child->_keys[B - 1]);
    _vals[idx] = mem::move(child->_vals[B - 1]);
    _children[idx + 1] = new_child;
    ++_len;
  }

  auto remove_from_leaf(usize idx) -> Option<V> {
    auto result = mem::move(_vals[idx]);

    // Shift elements left
    for (usize i = idx; i < _len - 1; ++i) {
      _keys[i] = mem::move(_keys[i + 1]);
      _vals[i] = mem::move(_vals[i + 1]);
    }
    --_len;

    return Option<V>{mem::move(result)};
  }

  auto remove_from_internal(usize idx) -> Option<V> {
    auto result = mem::move(_vals[idx]);

    // Find predecessor or successor
    if (_children[idx]->_len >= B) {
      // Use predecessor
      auto pred_key = _children[idx]->get_max_key();
      auto pred_val = _children[idx]->remove(pred_key);
      _keys[idx] = mem::move(pred_key);
      _vals[idx] = mem::move(pred_val.value());
    } else if (_children[idx + 1]->_len >= B) {
      // Use successor
      auto succ_key = _children[idx + 1]->get_min_key();
      auto succ_val = _children[idx + 1]->remove(succ_key);
      _keys[idx] = mem::move(succ_key);
      _vals[idx] = mem::move(succ_val.value());
    } else {
      // Merge children
      merge_children(idx);
      return _children[idx]->remove(_keys[idx]);
    }

    return Option<V>{mem::move(result)};
  }

  auto get_min_key() const -> K {
    if (_is_leaf) {
      return _keys[0];
    }
    return _children[0]->get_min_key();
  }

  auto get_max_key() const -> K {
    if (_is_leaf) {
      return _keys[_len - 1];
    }
    return _children[_len]->get_max_key();
  }

  void merge_children(usize idx) {
    auto left = _children[idx];
    auto right = _children[idx + 1];

    // Move the key down from this node
    left->_keys[left->_len] = mem::move(_keys[idx]);
    left->_vals[left->_len] = mem::move(_vals[idx]);

    // Move keys and values from right to left
    for (usize i = 0; i < right->_len; ++i) {
      left->_keys[left->_len + 1 + i] = mem::move(right->_keys[i]);
      left->_vals[left->_len + 1 + i] = mem::move(right->_vals[i]);
    }

    // Move children if internal nodes
    if (!left->_is_leaf) {
      for (usize i = 0; i <= right->_len; ++i) {
        left->_children[left->_len + 1 + i] = right->_children[i];
      }
    }

    left->_len += right->_len + 1;

    // Shift keys, values, and children in this node
    for (usize i = idx; i < _len - 1; ++i) {
      _keys[i] = mem::move(_keys[i + 1]);
      _vals[i] = mem::move(_vals[i + 1]);
      _children[i + 1] = _children[i + 2];
    }
    --_len;

    delete right;
  }

  void fix_child_underflow(usize idx) {
    if (idx > 0 && _children[idx - 1]->_len >= B) {
      // Borrow from left sibling
      borrow_from_left(idx);
    } else if (idx < _len && _children[idx + 1]->_len >= B) {
      // Borrow from right sibling
      borrow_from_right(idx);
    } else {
      // Merge with sibling
      if (idx > 0) {
        merge_children(idx - 1);
      } else {
        merge_children(idx);
      }
    }
  }

  void borrow_from_left(usize idx) {
    auto child = _children[idx];
    auto sibling = _children[idx - 1];

    // Shift child's elements right
    for (usize i = child->_len; i > 0; --i) {
      child->_keys[i] = mem::move(child->_keys[i - 1]);
      child->_vals[i] = mem::move(child->_vals[i - 1]);
    }

    if (!child->_is_leaf) {
      for (usize i = child->_len + 1; i > 0; --i) {
        child->_children[i] = child->_children[i - 1];
      }
    }

    // Move key from parent to child
    child->_keys[0] = mem::move(_keys[idx - 1]);
    child->_vals[0] = mem::move(_vals[idx - 1]);

    // Move key from sibling to parent
    _keys[idx - 1] = mem::move(sibling->_keys[sibling->_len - 1]);
    _vals[idx - 1] = mem::move(sibling->_vals[sibling->_len - 1]);

    if (!child->_is_leaf) {
      child->_children[0] = sibling->_children[sibling->_len];
    }

    ++child->_len;
    --sibling->_len;
  }

  void borrow_from_right(usize idx) {
    auto child = _children[idx];
    auto sibling = _children[idx + 1];

    // Move key from parent to child
    child->_keys[child->_len] = mem::move(_keys[idx]);
    child->_vals[child->_len] = mem::move(_vals[idx]);

    // Move key from sibling to parent
    _keys[idx] = mem::move(sibling->_keys[0]);
    _vals[idx] = mem::move(sibling->_vals[0]);

    if (!child->_is_leaf) {
      child->_children[child->_len + 1] = sibling->_children[0];
    }

    // Shift sibling's elements left
    for (usize i = 0; i < sibling->_len - 1; ++i) {
      sibling->_keys[i] = mem::move(sibling->_keys[i + 1]);
      sibling->_vals[i] = mem::move(sibling->_vals[i + 1]);
    }

    if (!sibling->_is_leaf) {
      for (usize i = 0; i < sibling->_len; ++i) {
        sibling->_children[i] = sibling->_children[i + 1];
      }
    }

    ++child->_len;
    --sibling->_len;
  }
};

}  // namespace sfc::collections::btree
