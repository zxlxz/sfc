#pragma once

#include "sfc/core.h"

namespace sfc::collections {

template <class K, class V, u16 B = 4>
class [[nodiscard]] BTree {
  struct Node;
  static_assert(B >= 2, "BTree: check `B>=2` failed");

  Node* _root = nullptr;
  usize _len = 0;

 public:
  BTree() = default;

  ~BTree() {
    if (_root) {
      delete _root;
    }
  }

  auto len() const -> usize {
    return _len;
  }

  auto get(const K& key) const -> Option<const V&> {
    if (_root == nullptr) {
      return {};
    }

    const auto [node, idx] = _root->find(key);
    if (!node) {
      return {};
    }
    return node->_vals[idx];
  }

  auto get_mut(const K& key) -> Option<V&> {
    if (_root == nullptr) {
      return {};
    }

    const auto [node, idx] = _root->find(key);
    if (!node) {
      return {};
    }
    return node->_vals[idx];
  }

  auto operator[](const K& key) const -> const V& {
    const auto opt = this->get(key);
    panicking::assert(opt, "collections::BTree::[]: key(=`{}`) not exists", key);
    return *opt;
  }

  auto operator[](const K& key) -> V& {
    const auto opt = this->get_mut(key);
    panicking::assert(opt, "collections::BTree::[]: key(=`{}`) not exists", key);
    return *opt;
  }

  auto try_insert(K key, V val) -> Option<V&> {
    if (_root == nullptr) {
      _root = new Node{false};
    }

    const auto ptr = _root->insert({static_cast<K&&>(key), static_cast<V&&>(val)});
    if (ptr) {
      return *ptr;
    }

    _len += 1;
    if (_root->_top) {
      _root = _root->_top;
    }
    return {};
  }

  auto insert(K key, V val) -> Option<V> {
    if (_root == nullptr) {
      _root = new Node{false};
    }

    const auto ptr = _root->insert({static_cast<K&&>(key), static_cast<V&&>(val)});
    if (ptr) {
      return mem::replace(*ptr, static_cast<V&&>(val));
    }

    _len += 1;
    if (_root->_top) {
      _root = _root->_top;
    }
    return {};
  }

  auto remove(const auto& key) -> Option<V> {
    if (_root == nullptr) {
      return {};
    }

    const auto ss = _root->find(key);
    if (!ss.node) {
      return {};
    }

    auto item = ss.node->remove(ss.index);
    _len -= 1;

    if (_root->_len == 0 && _root->_has_node) {
      auto old_root = _root;
      _root = _root->_nodes[0];
      _root->_top = nullptr;
      _root->_idx = 0;
      old_root->_has_node = false;
      delete old_root;
    }

    return static_cast<V&&>(item.val);
  }
};

template <class K, class V, u16 B>
struct BTree<K, V, B>::Node {
  struct Item {
    K key;
    V val;
  };

  struct FindResult {
    Node* node;
    usize index;
  };

  Node* _top = nullptr;
  u16 _idx = 0;
  u16 _len = 0;
  bool _has_node = false;

  union {
    struct {
      K _keys[2 * B - 1];
      V _vals[2 * B - 1];
    };
  };
  Node* _nodes[2 * B] = {nullptr};

 public:
  Node(bool has_node = false) : _has_node{has_node} {
    for (auto& node : _nodes) {
      node = nullptr;
    }
  }

  ~Node() {
    if (_len == 0) {
      return;
    }
    ptr::drop_in_place(_keys, _len);
    ptr::drop_in_place(_vals, _len);
    if (_has_node) {
      for (auto i = 0U; i <= _len; ++i) {
        delete _nodes[i];
      }
    }
  }

  auto find(const auto& key) const -> FindResult {
    const auto idx = this->position(key);
    if (idx < _len && key == _keys[idx]) {
      return {const_cast<Node*>(this), idx};
    }
    if (!_has_node) {
      return {};
    }
    return _nodes[idx]->find(key);
  }

  auto insert(Item&& t) -> V* {
    const auto idx = this->position(t.key);
    if (idx < _len && _keys[idx] == t.key) {
      return &_vals[idx];
    }

    if (!_has_node) {
      this->insert_at(idx, static_cast<Item&&>(t));
      return nullptr;
    }

    return _nodes[idx]->insert(static_cast<Item&&>(t));
  }

  auto remove(u32 idx) -> Item {
    panicking::assert(idx < _len,
                      "BTree::Node::remove: index(=`{}`) out of bounds(<`{}`)",
                      idx,
                      _len);

    auto res = Item{ptr::read(&_keys[idx]), ptr::read(&_vals[idx])};

    if (!_has_node) {
      ptr::shift_elements(_keys + idx + 1, _len - idx - 1, -1);
      ptr::shift_elements(_vals + idx + 1, _len - idx - 1, -1);
      --_len;
      return res;
    }

    auto lhs = _nodes[idx];
    if (lhs->_len >= B) {
      auto u = lhs->remove(lhs->_len - 1);
      new (&_keys[idx]) K{static_cast<K&&>(u.key)};
      new (&_vals[idx]) V{static_cast<V&&>(u.val)};
      return res;
    }

    auto rhs = _nodes[idx + 1];
    if (rhs->_len >= B) {
      auto u = rhs->remove(0);
      new (&_keys[idx]) K{static_cast<K&&>(u.key)};
      new (&_vals[idx]) V{static_cast<V&&>(u.val)};
      return res;
    }

    ptr::shift_elements(_keys + idx + 1, _len - idx - 1, -1);
    ptr::shift_elements(_vals + idx + 1, _len - idx - 1, -1);
    ptr::shift_elements(_nodes + idx + 2, _len - idx - 1, -1);
    _len -= 1;

    const auto pos = lhs->merge_with(static_cast<Item&&>(res), rhs);
    return lhs->remove(pos);
  }

 private:
  auto position(const auto& key) const -> u32 {
    auto idx = 0U;
    while (idx < _len && key > _keys[idx]) {
      ++idx;
    }
    return idx;
  }

  void insert_at(u32 idx, Item&& t, Node* child = nullptr) {
    ptr::shift_elements(_keys + idx, _len - idx, 1);
    ptr::shift_elements(_vals + idx, _len - idx, 1);
    new (&_keys[idx]) K{static_cast<K&&>(t.key)};
    new (&_vals[idx]) V{static_cast<V&&>(t.val)};

    if (child) {
      ptr::shift_elements(_nodes + idx + 1, _len - idx, 1);
      _nodes[idx + 1] = child;
      child->_top = this;
      child->_idx = static_cast<u16>(idx + 1);
    }
    ++_len;

    if (_len == 2 * B - 1) {
      this->split();
    }
  }

  void split() {
    if (_len < 2 * B - 1) {
      return;
    }

    _len = B - 1;

    if (_top == nullptr) {
      _top = new Node{true};
      _top->_nodes[0] = this;
    }

    auto rhs = new Node{_has_node};
    rhs->_len = B - 1;
    ptr::uninit_move(_keys + B, rhs->_keys, B - 1);
    ptr::uninit_move(_vals + B, rhs->_vals, B - 1);
    if (_has_node) {
      ptr::uninit_move(_nodes + B, rhs->_nodes, B);
      for (auto i = 0U; i < B; ++i) {
        rhs->_nodes[i]->_top = rhs;
        rhs->_nodes[i]->_idx = i;
      }
    }

    auto item = Item{ptr::read(&_keys[B - 1]), ptr::read(&_vals[B - 1])};
    _top->insert_at(_idx, static_cast<Item&&>(item), rhs);
  }

  auto merge_with(Item&& item, Node* rhs) -> u32 {
    const auto pos = _len;
    new (&_keys[_len]) K{static_cast<K&&>(item.key)};
    new (&_vals[_len]) V{static_cast<V&&>(item.val)};
    _len += 1;

    ptr::uninit_move(rhs->_keys, _keys + _len, rhs->_len);
    ptr::uninit_move(rhs->_vals, _vals + _len, rhs->_len);
    if (_has_node) {
      ptr::uninit_move(rhs->_nodes, _nodes + _len, rhs->_len + 1);
      for (auto i = _len; i < _len + rhs->_len + 1; ++i) {
        _nodes[i]->_top = this;
        _nodes[i]->_idx = i;
      }
    }
    _len += rhs->_len;
    rhs->_len = 0;
    delete rhs;

    return pos;
  }
};

}  // namespace sfc::collections
