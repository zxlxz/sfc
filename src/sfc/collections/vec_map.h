#pragma once

#include "sfc/alloc.h"

namespace sfc::collections::vec_map {

template <class K, class V>
class VecMap {
  struct Item {
    K key;
    V val;
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

  auto as_slice() const -> Slice<const Item> {
    return _items.as_slice();
  }

  auto get(const auto& key) const -> Option<const V&> {
    auto v = _items.as_slice();
    for (auto i = 0; i < v._len; ++i) {
      if (v._ptr[i].key == key) {
        return v._ptr[i].val;
      }
    }
    return {};
  }

  auto get_mut(const auto& key) -> Option<V&> {
    auto v = _items.as_mut_slice();
    for (auto i = 0; i < v._len; ++i) {
      if (v._ptr[i].key == key) {
        return v._ptr[i].val;
      }
    }
    return {};
  }

  auto operator[](const auto& key) const -> const V& {
    const auto idx = this->get(key);
    panicking::expect(idx, "VecMap::[]: key(=`{}`) not exists", key);
    return *idx;
  }

  auto operator[](const auto& key) -> V& {
    const auto opt = this->get_mut(key);
    panicking::expect(opt, "VecMap::[]: key(=`{}`) not exists", key);
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
  // trait: fmt::Display
  void fmt(auto& f) const {
    auto imp = f.debug_map();
    for (auto& item : _items) {
      imp.entry(item.key, item.value);
    }
  }

  // trait: serde::Serialize
  void serialize(auto& ser) const {
    auto imp = ser.serialize_map();
    for (auto& item : _items) {
      imp.serialize_field(item.key, item.value);
    }
  }
};

}  // namespace sfc::collections::vec_map

namespace sfc::collections {
using vec_map::VecMap;
}
