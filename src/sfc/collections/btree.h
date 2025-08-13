#pragma once

#include "sfc/core.h"

namespace sfc::collections {

template <class K, class V, u16 B = 4>
class [[nodiscard]] BTreeMap {
  struct Node;
  Node* _root = nullptr;
  usize _len = 0;

 public:
  BTreeMap() = default;

  ~BTreeMap() {
    if (_root) {
      delete _root;
    }
  }

  BTreeMap(BTreeMap&& other) noexcept : _root{other._root}, _len{other._len} {
    other._root = nullptr;
    other._len = 0;
  }

  BTreeMap& operator=(BTreeMap&& other) noexcept {
    if (this == &other) {
      return *this;
    }
    if (_root) {
      delete _root;
    }
    _root = mem::take(other._root);
    _len = mem::take(other._len);
    return *this;
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

  auto operator[](const K& key) -> V& {
    if (_root == nullptr) {
      return {};
    }

    const auto [node, idx] = _root->find(key);
    if (!node) {
      return {};
    }
    return node->_vals[idx];
  }

  void init() {
    if (_root != nullptr) {
      return;
    }
    _root = new Node{false};
  }

  auto try_insert(K key, V val) -> Option<V&> {
    if (_root == nullptr) {
      this->init();
    }

    if (auto ptr = _root->try_insert(mem::move(key), mem::move(val))) {
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
      this->init();
    }

    if (auto ptr = _root->try_insert(mem::move(key), mem::move(val))) {
      auto res = Option<V>{mem::move(*ptr)};
      *ptr = mem::move(val);
      return res;
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

    const auto [ptr, idx] = _root->find(key);
    if (!ptr) {
      return {};
    }

    auto res = Option{mem::move(ptr->_vals[idx])};
    ptr->remove(idx);
    _len -= 1;

    if (_root->_len == 0 && _root->_has_node) {
      auto old_root = _root;
      _root = _root->_nodes[0];
      _root->_top = nullptr;
      _root->_idx = 0;
      old_root->_has_node = false;
      delete old_root;
    }

    return res;
  }
};

template <class K, class V, u16 B>
struct BTreeMap<K, V, B>::Node {
  static_assert(B >= 2, "BTreeMap: check `B>=2` failed");
  static constexpr auto MAX_LEN = 2 * B - 1;
  static constexpr auto MIN_LEN = B - 1;

  struct Item {
    K key;
    V val;
  };

  struct FindResult {
    Node* ptr = nullptr;
    usize idx = 0;
  };

  Node* _top = nullptr;
  u16 _idx = 0;
  u16 _len = 0;
  bool _has_node = false;

  union {
    struct {
      K _keys[MAX_LEN];
      V _vals[MAX_LEN];
    };
  };
  Node* _nodes[2 * B] = {nullptr};

 public:
  Node(bool has_node) : _has_node{has_node} {}

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

  auto try_insert(K&& k, V&& v) -> V* {
    const auto idx = this->position(k);
    if (idx < _len && _keys[idx] == k) {
      return &_vals[idx];
    }

    if (_has_node) {
      return _nodes[idx]->try_insert(mem::move(k), mem::move(v));
    }

    this->insert_at(idx, mem::move(k), mem::move(v), nullptr);
    this->split();
    return nullptr;
  }

  void remove(u16 idx) {
    if (idx >= _len) {
      return;
    }

    if (!_has_node) {
      this->erase_at(idx);
      return;
    }

    auto lhs = _nodes[idx];
    if (lhs->_len > MIN_LEN) {
      _keys[idx] = mem::move(lhs->_keys[lhs->_len - 1]);
      _vals[idx] = mem::move(lhs->_vals[lhs->_len - 1]);
      return lhs->remove(lhs->_len - 1);
    }

    auto rhs = _nodes[idx + 1];
    if (rhs->_len > MIN_LEN) {
      _keys[idx] = mem::move(rhs->_keys[0]);
      _vals[idx] = mem::move(rhs->_vals[0]);
      return rhs->remove(0);
    }

    this->merge_at(idx);
    return lhs->remove(MIN_LEN);
  }

  void split() {
    if (_len != MAX_LEN) {
      return;
    }

    if (_top == nullptr) {
      _top = new Node{true};
      _top->_nodes[0] = this;
    }

    auto rhs = new Node{_has_node};
    rhs->append(*this, B);

    auto k = ptr::read(&_keys[MIN_LEN]);
    auto v = ptr::read(&_vals[MIN_LEN]);
    _top->insert_at(_idx, mem::move(k), mem::move(v), rhs);
    _len -= 1;

    _top->split();
  }

  void rebalance() {
    if (_top == nullptr || _len >= MIN_LEN) {
      return;
    }

    if (_idx > 0) {
      auto lhs = _top->_nodes[_idx - 1];
      auto rhs = this;
      if (lhs->_len + rhs->_len < MAX_LEN) {
        return _top->merge_at(_idx - 1);
      }
    }

    if (_idx < _top->_len) {
      auto lhs = this;
      auto rhs = _top->_nodes[_idx + 1];
      if (lhs->_len + rhs->_len < MAX_LEN) {
        return _top->merge_at(_idx);
      }
    }
  }

 private:
  auto position(const auto& key) const -> u16 {
    auto idx = 0U;
    while (idx < _len && key > _keys[idx]) {
      ++idx;
    }
    return idx;
  }

  void insert_at(u16 idx, K&& k, V&& v, Node* c) {
    ptr::shift_elements_right(_keys + idx, _len - idx, 1);
    ptr::write(&_keys[idx], mem::move(k));

    ptr::shift_elements_right(_vals + idx, _len - idx, 1);
    ptr::write(&_vals[idx], mem::move(v));
    _len += 1;

    if (c) {
      c->_top = this;
      c->_idx = idx + 1;
      for (auto i = _len; i > idx + 1; --i) {
        _nodes[i] = _nodes[i - 1];
        _nodes[i]->_idx = i;
      }
      _nodes[idx + 1] = c;
    }
  }

  void erase_at(u16 idx) {
    if (idx >= _len) {
      return;
    }

    _keys[idx].~K();
    ptr::shift_elements_left(_keys + idx + 1, _len - idx - 1, 1);

    _vals[idx].~V();
    ptr::shift_elements_left(_vals + idx + 1, _len - idx - 1, 1);

    if (_has_node) {
      for (auto i = idx + 1U; i < _len; ++i) {
        _nodes[i] = _nodes[i + 1];
        _nodes[i]->_idx = i;
      }
    }
    _len -= 1;

    this->rebalance();
  }

  void merge_at(u16 idx) {
    auto lhs = _nodes[idx];
    auto rhs = _nodes[idx + 1];
    ptr::write(&lhs->_keys[lhs->_len], mem::move(_keys[idx]));
    ptr::write(&lhs->_vals[lhs->_len], mem::move(_vals[idx]));
    lhs->_len += 1;
    lhs->append(*rhs, 0);
    delete rhs;

    this->erase_at(idx);
  }

  void append(Node& src, u16 idx) {
    if (_len >= MAX_LEN) {
      return;
    }

    const auto cnt = static_cast<u16>(src._len - idx);
    ptr::uninit_move(src._keys + idx, _keys + _len, cnt);
    ptr::drop_in_place(src._keys + idx, cnt);

    ptr::uninit_move(src._vals + idx, _vals + _len, cnt);
    ptr::drop_in_place(src._vals + idx, cnt);

    if (_has_node) {
      for (auto i = _len, j = idx; i < _len + cnt + 1; ++i, ++j) {
        _nodes[i] = src._nodes[j];
        _nodes[i]->_top = this;
        _nodes[i]->_idx = i;
      }
    }

    src._len -= cnt;
    _len += cnt;
  }
};

}  // namespace sfc::collections
