#pragma once

#include "sfc/collections/hash_tbl.h"

namespace sfc::collections::hash_map {

template <class K, class V>
class HashMap {
  struct Entry {
    K key;
    V val;
  };
  hash_tbl::HashTbl<Entry> _inn;

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
    return _inn.capacity();
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
    return const_cast<V&>(p->val);
  }

  auto try_insert(K key, V val) noexcept -> Option<V&> {
    const auto p = _inn.try_insert({static_cast<K&&>(key), static_cast<V&&>(val)});
    if (!p) {
      return {};
    }
    return p->val;
  }

  auto insert(K key, V val) noexcept -> Option<V> {
    if (auto ptr = _inn.try_insert({static_cast<K&&>(key), static_cast<V&&>(val)})) {
      auto old_val = mem::replace(ptr->val, static_cast<V&&>(val));
      return Option{mem::move(old_val)};
    }
    return {};
  }

  auto remove(const K& key) -> Option<V> {
    const auto p = const_cast<Entry*>(_inn.search(key));
    if (!p) {
      return {};
    }
    auto res = Option{static_cast<V&&>(p->val)};
    _inn.erase(p);
    return res;
  }

  void clear() {
    _inn.clear();
  }

 public:
  // trait: fmt::Display
  void fmt(auto& f) const {
    auto imp = f.debug_map();
    _inn.for_each([&](const auto& entry) { imp.entry(entry.key, entry.val); });
  }

  // trait: serde::Serialize
  void serialize(auto& ser) const {
    auto map = ser.serialize_map();
    _inn.for_each([&](const auto& entry) { map.serialize_entry(entry.key, entry.val); });
    map.end();
  }

  // trait: serde::Deserialize
  template <class D, class E = typename D::Error>
  static auto deserialize(D& des) -> Result<HashMap, E> {
    auto res = HashMap{};
    auto visit = [&](auto& map) -> Result<void, E> {
      while (map.has_next()) {
        auto key = _TRY(map.template next_key<K>());
        auto val = _TRY(map.template next_value<V>());
        res.insert(key, static_cast<V&&>(val));
      }
      return {};
    };
    _TRY(des.deserialize_map(visit));
    return res;
  }
};

}  // namespace sfc::collections::hash_map

namespace sfc::collections {
using hash_map::HashMap;
}
