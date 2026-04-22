#pragma once

#include "sfc/collections/hash_tbl.h"

namespace sfc::collections::hash_map {

using hash_tbl::HashTbl;

template <class K, class V>
class HashMap {
  struct Entry {
    K key;
    V val;
  };
  HashTbl<Entry> _inn;

 public:
  static auto with_capacity(usize min_capacity) -> HashMap {
    auto res = HashMap{};
    res._inn.reserve(min_capacity);
    return res;
  }

  auto len() const noexcept -> usize {
    return _inn.len();
  }

  auto capacity() const noexcept -> usize {
    return _inn.cap();
  }

  void reserve(usize additional) {
    _inn.reserve(additional);
  }

 public:
  auto contains_key(const auto& key) const noexcept -> bool {
    const auto p = _inn.search(key);
    return p != nullptr;
  }

  auto get(const auto& key) const noexcept -> Option<const V&> {
    const auto p = _inn.search(key);
    if (!p) {
      return {};
    }
    return p->val;
  }

  auto get_mut(const auto& key) noexcept -> Option<V&> {
    const auto p = _inn.search(key);
    if (!p) {
      return {};
    }
    return p->val;
  }

  auto try_insert(K key, V val) noexcept -> Option<V&> {
    const auto p = _inn.try_insert({static_cast<K&&>(key), static_cast<V&&>(val)});
    if (!p) {
      return {};
    }
    return p->val;
  }

  auto insert(K key, V val) noexcept -> Option<V> {
    const auto p = _inn.try_insert({static_cast<K&&>(key), static_cast<V&&>(val)});
    if (!p) {
      return {};
    }
    return mem::replace(p->val, static_cast<V&&>(val));
  }

  auto remove(const K& key) -> Option<V> {
    auto x = _inn.remove(key);
    if (!x) return {};
    return static_cast<V&&>(x->val);
  }

  void clear() {
    _inn.clear();
  }

 public:
  // trait: fmt::Display
  void fmt(auto& f) const {
    auto imp = f.debug_map();
    _inn.iter().for_each([&](const Entry& entry) { imp.entry(entry.key, entry.val); });
  }

  // trait: serde::Serialize
  void serialize(auto& ser) const {
    auto imp = ser.serialize_map();
    _inn.iter().for_each([&](const Entry& entry) { imp.serialize_entry(entry.key, entry.val); });
  }

  // trait: serde::Deserialize
  template <class D>
  static auto deserialize(D& des) {
    auto visit = [&](auto& map) { return map.template collect<HashMap, K, V>(); };
    return des.deserialize_map(visit);
  }
};

}  // namespace sfc::collections::hash_map

namespace sfc::collections {
using hash_map::HashMap;
}
