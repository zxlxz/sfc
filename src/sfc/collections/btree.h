#pragma once

#include "sfc/alloc.h"

namespace sfc::collections {

template <class K, class V, u16 B = 6>
class BTreeNode {
  using A = alloc::Global;

  static_assert(B >= 2, "BTreeMap: check `B>=2` failed");
  static constexpr auto MAX_LEN = 2 * B - 1;
  static constexpr auto MIN_LEN = B - 1;

  BTreeNode* _top = nullptr;
  u16 _idx = 0;
  u16 _len = 0;
  bool _has_edge = false;

  union {
    K _keys[MAX_LEN];
  };
  union {
    V _vals[MAX_LEN];
  };
  BTreeNode* _edges[2 * B] = {nullptr};

 public:
  explicit BTreeNode(bool has_edge = false) : _has_edge{has_edge} {}

  ~BTreeNode() {
    this->clear();
  }

  auto search(const auto& key) -> V* {
    const auto idx = this->find_idx(key);
    if (idx < _len && _keys[idx] == key) {
      return &_vals[idx];
    }
    if (!_has_edge) {
      return nullptr;
    }
    return _edges[idx]->search(key);
  }

  auto search_or_insert(K&& key, V&& val) -> V* {
    const auto idx = this->find_idx(key);
    if (idx < _len && _keys[idx] == key) {
      return &_vals[idx];
    }

    if (!_has_edge) {
      this->insert_at(idx, static_cast<K&&>(key), static_cast<V&&>(val));
      return nullptr;
    }

    return _edges[idx]->search_or_insert(static_cast<K&&>(key), static_cast<V&&>(val));
  }

  auto remove(const auto& key, auto&& f) -> bool {
    const auto idx = this->find_idx(key);
    if (idx < _len && _keys[idx] == key) {
      f(_vals[idx]);
      this->erase_at(idx);
      if (_len < MIN_LEN) {
        this->rebalance();
      }
      return true;
    }
    if (!_has_edge) {
      return false;
    }
    return _edges[idx]->remove(key, f);
  }

  void clear() {
    ptr::drop_in_place(_keys, _len);
    ptr::drop_in_place(_vals, _len);

    if (_has_edge) {
      for (auto i = 0U; i <= _len; ++i) {
        BTreeNode::xdel(_edges[i]);
      }
    }
    _len = 0;
  }

 private:
  static auto xnew(bool has_edge) -> BTreeNode {
    const auto size = (has_edge) ? sizeof(BTreeNode) : sizeof(BTreeNode) - sizeof(_edges);
    const auto ptr = static_cast<BTreeNode*>(A::alloc({size, alignof(BTreeNode)}));
    return new (ptr) BTreeNode{has_edge};
  }

  static auto xdel(BTreeNode* ptr) -> void {
    if (ptr == nullptr) {
      return;
    }
    ptr->clear();
    A::dealloc(ptr);
  }

  auto find_idx(const auto& key) const -> u16 {
    auto idx = u16{0};
    while (idx < _len && _keys[idx] < key) {
      idx += 1;
    }
    return idx;
  }

  void insert_at(u16 idx, K&& key, V&& val, BTreeNode* child = nullptr) {
    ptr::shift_elements_right(_keys + idx, _len - idx, 1);
    ptr::write(&_keys[idx], static_cast<K&&>(key));

    ptr::shift_elements_right(_vals + idx, _len - idx, 1);
    ptr::write(&_vals[idx], static_cast<V&&>(val));
    _len += 1;

    if (child) {
      ptr::shift_elements_right(_edges + idx + 1, _len - idx - 1, 1);
      _edges[idx + 1] = child;
      child->_top = this;

      for (auto i = idx + 1U; i <= _len; ++i) {
        _edges[i]->_top = this;
      }
    }

    if (_len == MAX_LEN) {
      this->split();
    }
  }

  void erase_at(u16 idx) {
    if (!_has_edge) {
      ptr::pop_front(_keys + idx, _len - idx);
      ptr::pop_front(_vals + idx, _len - idx);
      _len -= 1;

      if (_len < MIN_LEN) {
        this->rebalance();
      }
      return;
    }

    auto leaf = _edges[idx + 1];
    while (leaf->_has_edge) {
      leaf = leaf->_edges[0];
    }
    _keys[idx] = static_cast<K&&>(leaf->_keys[0]);
    _vals[idx] = static_cast<V&&>(leaf->_vals[0]);
    leaf->erase_at(0);
  }

  void merge_at(u16 idx) {
    auto lhs = _edges[idx];
    auto rhs = _edges[idx + 1];

    ptr::write(&lhs->_keys[lhs->_len], static_cast<K&&>(_keys[idx]));
    ptr::write(&lhs->_vals[lhs->_len], static_cast<V&&>(_vals[idx]));
    lhs->_len += 1;

    lhs->append_elements(*rhs, 0);
    BTreeNode::xdel(rhs);

    this->erase_at(idx);
  }

  void append_elements(BTreeNode& src, u16 idx) {
    const auto cnt = src._len - idx;
    ptr::uninit_move(src._keys + idx, _keys + _len, cnt);
    ptr::uninit_move(src._vals + idx, _vals + _len, cnt);

    if (_has_edge) {
      ptr::uninit_move(src._edges + idx, _edges + _len, cnt + 1);
      for (auto i = _len; i <= _len + cnt; ++i) {
        _edges[i]->_top = this;
        _edges[i]->_idx = i;
      }
    }

    _len += cnt;
    src._len -= cnt;
  }

  void split() {
    auto mid_key = ptr::read(&_keys[MIN_LEN]);
    auto mid_val = ptr::read(&_vals[MIN_LEN]);

    auto rhs = BTreeNode::xnew(_has_edge);
    _len = MIN_LEN;
    rhs->_len = MIN_LEN;
    ptr::uninit_move(_keys + B, rhs->_keys, MIN_LEN);
    ptr::uninit_move(_vals + B, rhs->_vals, MIN_LEN);
    if (_has_edge) {
      ptr::uninit_move(_edges + B, rhs->_edges, MIN_LEN);
      for (auto i = 0U; i <= MIN_LEN; ++i) {
        rhs->_edges[i]->_top = rhs;
        rhs->_edges[i]->_idx = i;
      }
    }

    if (!_top) {
      _top = BTreeNode::xnew(true);
      _top->_edges[0] = this;
    }

    rhs->_top = _top;
    _top->insert_at(_idx, static_cast<K&&>(mid_key), static_cast<V&&>(mid_val), rhs);
    if (_top->_len >= MAX_LEN) {
      _top->split();
    }
  }

  void rebalance() {
    if (_top == nullptr) {
      return;
    }

    if (_idx > 0) {
      if (auto lhs = _top->_edges[_idx - 1]; lhs->_len > MIN_LEN) {
        this->borrow_left(*lhs);
      } else {
        _top->merge_at(_idx - 1);
        return;
      }
    }

    if (_idx < _top->_len) {
      if (auto rhs = _top->_edges[_idx + 1]; rhs->_len > MIN_LEN) {
        this->borrow_right(*rhs);
      } else {
        _top->merge_at(_idx);
      }
    }
  }

  void borrow_left(BTreeNode& lhs) {
    auto lhs_key = ptr::read(&lhs._keys[lhs._len - 1]);
    auto lhs_val = ptr::read(&lhs._vals[lhs._len - 1]);
    auto lhs_edge = _has_edge ? lhs._edges[lhs._len] : nullptr;
    lhs._len -= 1;

    auto& top_key = _top->_keys[_idx - 1];
    auto& top_val = _top->_vals[_idx - 1];
    ptr::push_front(_keys, _len, static_cast<K&&>(top_key));
    ptr::push_front(_vals, _len, static_cast<V&&>(top_val));
    top_key = static_cast<K&&>(lhs_key);
    top_val = static_cast<V&&>(lhs_val);

    if (lhs_edge) {
      lhs_edge->_top = this;
      lhs_edge->_idx = 0;
      ptr::push_front(_edges, _len + 1, lhs_edge);
    }
    _len += 1;
  }

  void borrow_right(BTreeNode& rhs) {
    auto rhs_key = ptr::pop_front(rhs._keys, rhs._len);
    auto rhs_val = ptr::pop_front(rhs._vals, rhs._len);
    auto rhs_edge = _has_edge ? rhs._edges[rhs._len + 1] : nullptr;
    rhs._len -= 1;

    auto& top_key = _top->_keys[_idx];
    auto& top_val = _top->_vals[_idx];
    ptr::write(&_keys[_len], static_cast<K&&>(top_key));
    ptr::write(&_vals[_len], static_cast<V&&>(top_val));
    top_key = static_cast<K&&>(rhs_key);
    top_val = static_cast<V&&>(rhs_val);

    if (rhs_edge) {
      rhs_edge->_top = this;
      rhs_edge->_idx = _len;
      _edges[_len] = rhs_edge;
    }
    _len += 1;
  }
};

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

}  // namespace sfc::collections
