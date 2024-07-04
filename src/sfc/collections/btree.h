#pragma once

#include "sfc/alloc.h"

namespace sfc::collections {

template <class K, class V, u16 B>
struct BTreeNode;

template <class K, class V, u16 B = 6>
struct BTreeLeaf {
  using A = alloc::Global;

  BTreeNode<K, V, B>* _top = nullptr;
  u16 _idx = 0;
  u16 _len = 0;
  bool _is_leaf = true;

  union {
    K _keys[2 * B - 1];
  };
  union {
    V _vals[2 * B - 1];
  };

 public:
  BTreeLeaf() = default;

  ~BTreeLeaf() {
    if (_len == 0) {
      return;
    }
    ptr::drop_in_place(_keys, _len);
    ptr::drop_in_place(_vals, _len);
  }

  BTreeLeaf(const BTreeLeaf&) = delete;

  BTreeLeaf& operator=(const BTreeLeaf&) = delete;
};

template <class K, class V, u16 B = 6>
struct BTreeNode {
  using A = alloc::Global;
  using BTreeLeaf = collections::BTreeLeaf<K, V, B>;

  static_assert(B >= 2, "BTreeMap: check `B>=2` failed");
  static constexpr auto MAX_LEN = 2 * B - 1;
  static constexpr auto MIN_LEN = B - 1;

  BTreeNode<K, V, B>* _top = nullptr;
  u16 _idx = 0;
  u16 _len = 0;
  bool _is_leaf = false;

  union {
    K _keys[2 * B - 1];
  };
  union {
    V _vals[2 * B - 1];
  };

  BTreeNode* _edges[2 * B] = {nullptr};

 public:
  BTreeNode() = default;

  ~BTreeNode() {
    if (_len == 0) {
      return;
    }
    ptr::drop_in_place(_keys, _len);
    ptr::drop_in_place(_vals, _len);
    for (auto i = 0U; i <= _len; ++i) {
      BTreeNode::xdel(_edges[i]);
    }
  }

  BTreeNode(const BTreeNode&) = delete;

  BTreeNode& operator=(const BTreeNode&) = delete;

  static auto xnew(bool is_leaf) -> BTreeNode* {
    if (is_leaf) {
      const auto ptr = A::alloc({sizeof(BTreeLeaf), alignof(BTreeLeaf)});
      new (static_cast<BTreeLeaf*>(ptr)) BTreeLeaf{};
      return static_cast<BTreeNode*>(ptr);
    } else {
      const auto ptr = A::alloc({sizeof(BTreeNode), alignof(BTreeNode)});
      return new (static_cast<BTreeNode*>(ptr)) BTreeNode{};
    }
  }

  static auto xdel(BTreeNode* ptr) -> void {
    if (ptr == nullptr) {
      return;
    }

    if (ptr->_is_leaf) {
      auto p = static_cast<BTreeLeaf*>(static_cast<void*>(ptr));
      p->~BTreeLeaf();
      A::dealloc(p, {sizeof(BTreeLeaf), alignof(BTreeLeaf)});
    } else {
      ptr->~BTreeNode();
      A::dealloc(ptr, {sizeof(BTreeNode), alignof(BTreeNode)});
    }
  }

  auto search(const auto& key) -> V* {
    const auto idx = this->find_idx(key);
    if (idx < _len && _keys[idx] == key) {
      return &_vals[idx];
    }
    if (_is_leaf) {
      return nullptr;
    }
    return _edges[idx]->search(key);
  }

  auto search_or_insert(K&& key, V&& val) -> V* {
    const auto idx = this->find_idx(key);
    if (idx < _len && _keys[idx] == key) {
      return &_vals[idx];
    }

    if (_is_leaf) {
      this->insert_at(idx, static_cast<K&&>(key), static_cast<V&&>(val));
      if (this->_len == MAX_LEN) {
        this->split();
      }
      return nullptr;
    }

    return _edges[idx]->search_or_insert(static_cast<K&&>(key), static_cast<V&&>(val));
  }

  auto remove(const auto& key, auto&& f) -> bool {
    const auto idx = this->find_idx(key);
    if (idx < _len && _keys[idx] == key) {
      f(_vals[idx]);
      auto ptr = this->erase_at(idx);
      if (ptr->_len < MIN_LEN) {
        ptr->rebalance();
      }
      return true;
    }

    if (_is_leaf) {
      return false;
    }

    return _edges[idx]->remove(key, f);
  }

 private:
  auto find_idx(const auto& key) const -> u16 {
    auto idx = u16{0};
    while (idx < _len && _keys[idx] < key) {
      idx += 1;
    }
    return idx;
  }

  void insert_at(u16 idx, K&& key, V&& val, BTreeNode* rhs = nullptr) {
    ptr::push_front(_keys + idx, _len - idx, static_cast<K&&>(key));
    ptr::push_front(_vals + idx, _len - idx, static_cast<V&&>(val));
    _len += 1;

    if (rhs) {
      ptr::push_front(_edges + idx + 1, _len - idx - 1, rhs);
      rhs->_top = this;

      for (auto i = idx + 1U; i <= _len; ++i) {
        _edges[i]->_idx = i;
      }
    }
  }

  void split() {
    if (_len < MAX_LEN) {
      return;
    }

    if (!_top) {
      _top = BTreeNode::xnew(false);
      _top->_edges[0] = this;
      _idx = 0;
    }

    auto rhs = BTreeNode::xnew(_is_leaf);
    rhs->_top = _top;
    rhs->append_elements(*this, MIN_LEN + 1);

    auto mid_key = ptr::read(&_keys[MIN_LEN]);
    auto mid_val = ptr::read(&_vals[MIN_LEN]);
    _len -= 1;

    _top->insert_at(_idx, static_cast<K&&>(mid_key), static_cast<V&&>(mid_val), rhs);
    if (_top->_len == MAX_LEN) {
      _top->split();
    }
  }

  void rebalance() {
    if (_top == nullptr || _len >= MIN_LEN) {
      return;
    }

    if (_idx > 0) {
      if (auto lhs = _top->_edges[_idx - 1]; lhs->_len > MIN_LEN) {
        return this->borrow_left(*lhs);
      }
    }

    if (_idx < _top->_len) {
      if (auto rhs = _top->_edges[_idx + 1]; rhs->_len > MIN_LEN) {
        return this->borrow_right(*rhs);
      }
    }

    _top->merge_at(_idx == 0 ? 0 : _idx - 1);
    if (_top->_len < MIN_LEN) {
      _top->rebalance();
    }
  }

  auto erase_at(u16 idx) -> BTreeNode* {
    if (idx >= _len) {
      return nullptr;
    }

    if (_is_leaf) {
      ptr::pop_front(_keys + idx, _len - idx);
      ptr::pop_front(_vals + idx, _len - idx);
      _len -= 1;
      return this;
    }

    auto leaf = _edges[idx];
    while (!leaf->_is_leaf) {
      leaf = leaf->_edges[leaf->_len];
    }
    _keys[idx] = ptr::read(&leaf->_keys[leaf->_len - 1]);
    _vals[idx] = ptr::read(&leaf->_vals[leaf->_len - 1]);
    leaf->_len -= 1;
    return leaf;
  }

  void borrow_left(BTreeNode& lhs) {
    auto lhs_key = ptr::read(&lhs._keys[lhs._len - 1]);
    auto lhs_val = ptr::read(&lhs._vals[lhs._len - 1]);
    auto lhs_edge = !_is_leaf ? lhs._edges[lhs._len] : nullptr;
    lhs._len -= 1;

    auto& top_key = _top->_keys[_idx - 1];
    auto& top_val = _top->_vals[_idx - 1];
    ptr::push_front(_keys, _len, static_cast<K&&>(top_key));
    ptr::push_front(_vals, _len, static_cast<V&&>(top_val));
    top_key = static_cast<K&&>(lhs_key);
    top_val = static_cast<V&&>(lhs_val);

    if (lhs_edge) {
      lhs_edge->_top = this;
      ptr::push_front(_edges, _len + 1, lhs_edge);
      for (auto i = 0U; i <= _len + 1; ++i) {
        _edges[i]->_idx = i;
      }
    }
    _len += 1;
  }

  void borrow_right(BTreeNode& rhs) {
    auto rhs_key = ptr::pop_front(rhs._keys, rhs._len);
    auto rhs_val = ptr::pop_front(rhs._vals, rhs._len);
    auto rhs_edge = !_is_leaf ? ptr::pop_front(rhs._edges, rhs._len + 1) : nullptr;
    rhs._len -= 1;
    if (!_is_leaf) {
      for (auto i = 0U; i <= rhs._len; ++i) {
        rhs._edges[i]->_idx = i;
      }
    }

    auto& top_key = _top->_keys[_idx];
    auto& top_val = _top->_vals[_idx];
    ptr::write(&_keys[_len], static_cast<K&&>(top_key));
    ptr::write(&_vals[_len], static_cast<V&&>(top_val));
    top_key = static_cast<K&&>(rhs_key);
    top_val = static_cast<V&&>(rhs_val);
    _len += 1;

    if (rhs_edge) {
      rhs_edge->_top = this;
      rhs_edge->_idx = _len;
      _edges[_len] = rhs_edge;
    }
  }

  void merge_at(u16 idx) {
    auto lhs = _edges[idx];
    auto rhs = _edges[idx + 1];
    auto key = ptr::pop_front(_keys + idx, _len - idx);
    auto val = ptr::pop_front(_vals + idx, _len - idx);
    ptr::pop_front(_edges + idx + 1, _len - idx);

    _len -= 1;
    for (auto i = idx + 1; i <= _len; ++i) {
      _edges[i]->_idx = i;
    }

    ptr::write(&lhs->_keys[lhs->_len], static_cast<K&&>(key));
    ptr::write(&lhs->_vals[lhs->_len], static_cast<V&&>(val));
    lhs->_len += 1;
    lhs->append_elements(*rhs, 0);
    BTreeNode::xdel(rhs);
  }

  void append_elements(BTreeNode& src, u16 idx) {
    const auto cnt = src._len - idx;
    ptr::uninit_move(src._keys + idx, _keys + _len, cnt);
    ptr::uninit_move(src._vals + idx, _vals + _len, cnt);

    if (!_is_leaf) {
      ptr::uninit_move(src._edges + idx, _edges + _len, cnt + 1);
      for (auto i = _len; i <= _len + cnt; ++i) {
        _edges[i]->_top = this;
        _edges[i]->_idx = i;
      }
    }

    _len += cnt;
    src._len -= cnt;
  }
};

template <class K, class V, u16 B = 6>
class [[nodiscard]] BTreeMap {
  using Node = BTreeNode<K, V, B>;

  Node* _root = nullptr;
  usize _len = 0;

 public:
  BTreeMap() = default;

  ~BTreeMap() {
    if (_root != nullptr) {
      Node::xdel(_root);
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
    if (_root != nullptr) {
      Node::xdel(_root);
    }
    _root = mem::take(other._root);
    _len = mem::take(other._len);
    return *this;
  }

  auto len() const -> usize {
    return _len;
  }

  auto get(const auto& key) const -> Option<const V&> {
    if (_root == nullptr) {
      return {};
    }

    if (auto ptr = _root->search(key)) {
      return *ptr;
    }
    return {};
  }

  auto get_mut(const auto& key) -> Option<V&> {
    if (_root == nullptr) {
      return {};
    }

    if (auto ptr = _root->search(key)) {
      return *ptr;
    }
    return {};
  }

  auto try_insert(K key, V val) -> Option<V&> {
    if (_root == nullptr) {
      _root = Node::xnew(true);
      _root->search_or_insert(static_cast<K&&>(key), static_cast<V&&>(val));
      _len += 1;
      return {};
    }

    if (auto ptr = _root->search_or_insert(static_cast<K&&>(key), static_cast<V&&>(val))) {
      return *ptr;
    }

    _len += 1;
    while (auto top = _root->_top) {
      _root = top;
    }

    return {};
  }

  auto insert(K key, V val) -> Option<V> {
    if (_root == nullptr) {
      _root = Node::xnew(true);
      _root->search_or_insert(static_cast<K&&>(key), static_cast<V&&>(val));
      _len += 1;
      return {};
    }

    if (auto ptr = _root->search_or_insert(static_cast<K&&>(key), static_cast<V&&>(val))) {
      return mem::replace(*ptr, static_cast<V&&>(val));
    }

    _len += 1;
    while (auto top = _root->_top) {
      _root = top;
    }
    return {};
  }

  auto remove(const auto& key) -> Option<V> {
    if (_root == nullptr) {
      return {};
    }

    auto res = Option<V>{};
    if (!_root->remove(key, [&res](auto& val) { res = static_cast<V&&>(val); })) {
      return {};
    }

    _len -= 1;
    if (_root->_len == 0 && !_root->_is_leaf) {
      const auto old_root = mem::replace(_root, _root->_edges[0]);
      Node::xdel(old_root);
      if (_root) {
        _root->_top = nullptr;
        _root->_idx = 0;
      }
    }

    return res;
  }
};

}  // namespace sfc::collections
