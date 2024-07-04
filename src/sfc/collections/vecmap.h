#pragma once

#include "sfc/alloc.h"

namespace sfc::collections {

template <class K, class V>
class VecMap {
  struct Item {
    K key;
    V value;

    auto operator==(const auto& key) const -> bool {
      return this->key == key;
    }
  };

  Vec<Item> _items = {};

 public:
  VecMap() noexcept = default;

  ~VecMap() = default;

  VecMap(VecMap&&) noexcept = default;

  auto operator=(VecMap&&) noexcept -> VecMap& = default;

  auto len() const -> usize {
    return _items.len();
  }

  auto capacity() const -> usize {
    return _items.capacity();
  }

  auto is_empty() const -> bool {
    return _items.is_empty();
  }

  auto as_slice() const -> slice::Slice<const Item> {
    return _items.as_slice();
  }

  auto get(const auto& key) const -> Option<const V&> {
    const auto idx = _items.find(key);
    if (!idx) {
      return {};
    }
    return _items[*idx].value;
  }

  auto get_mut(const auto& key) -> Option<V&> {
    const auto idx = _items.find(key);
    if (!idx) {
      return {};
    }
    return _items[*idx].value;
  }

  auto operator[](const auto& key) const -> const V& {
    const auto idx = this->get(key);
    panicking::assert(idx, "VecMap::[]: key(=`{}`) not exists", key);
    return *idx;
  }

  auto operator[](const auto& key) -> V& {
    const auto opt = this->get_mut(key);
    panicking::assert(opt, "VecMap::[]: key(=`{}`) not exists", key);
    return *opt;
  }

  auto insert(K key, V val) -> Option<V> {
    if (auto pos = this->get_mut(key)) {
      return mem::replace(*pos, static_cast<V&&>(val));
    }
    _items.push({static_cast<K&&>(key), static_cast<V&&>(val)});
    return {};
  }

  auto try_insert(K key, V val) -> Option<V&> {
    if (auto pos = this->get_mut(key)) {
      return pos;
    }
    _items.push({static_cast<K&&>(key), static_cast<V&&>(val)});
    return {};
  }

  auto remove(const K& key) -> Option<V> {
    const auto idx = _items.find(key);
    if (!idx) {
      return {};
    }

    auto res = _items.swap_remove(*idx);
    return static_cast<V&&>(res.value);
  }

  void clear() {
    _items.clear();
  }

  void reserve(usize amt) {
    _items.reserve(amt);
  }

 public:
  void fmt(auto& f) const {
    auto imp = f.debug_map();
    for (auto& item : _items) {
      imp.entry(item.key, item.value);
    }
  }

  auto serialize(auto& s) const {
    auto dict = s.new_dict();
    for (auto& item : _items) {
      dict.insert(item.key, s.ser(item.value));
    }
    return dict;
  }
};

}  // namespace sfc::collections
