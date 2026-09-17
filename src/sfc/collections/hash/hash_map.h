#pragma once

#include "sfc/collections/hash/hash_tbl.h"

namespace sfc::collections::hash {

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
    if (auto* p = _inn.search(key)) {
      return p->val;
    }
    return {};
  }

  auto get_mut(const auto& key) noexcept -> Option<V&> {
    if (auto* p = _inn.search(key)) {
      return p->val;
    }
    return {};
  }

  auto try_insert(K key, V val) noexcept -> Option<V&> {
    auto tmp = Entry{mem::move(key), mem::move(val)};
    if (auto ref = _inn.try_insert(mem::move(tmp))) {
      return ref->val;
    }
    return {};
  }

  auto insert(K key, V val) noexcept -> Option<V> {
    auto tmp = Entry{mem::move(key), mem::move(val)};
    if (auto res = _inn.insert(mem::move(tmp))) {
      return mem::move(res->val);
    }
    return {};
  }

  auto remove(const K& key) -> Option<V> {
    return _inn.remove(key).map([](auto entry) { return mem::move(entry.val); });
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
    auto imp = ser.serialize_obj();
    _inn.iter().for_each([&](const Entry& entry) { imp.serialize_entry(entry.key, entry.val); });
  }

  // trait: serde::Deserialize
  template <class D>
  static auto deserialize(D& des) -> D::template Result<HashMap> {
    auto res = HashMap{};
    auto imp = _TRY(des.deserialize_dict());
    _TRY(imp.template for_each<V>([&](auto key, V val) {
      if constexpr (trait::same_<K, String>) {
        res.insert(String::from(key), mem::move(val));
      } else {
        res.insert(key, mem::move(val));
      }
    }));
    return res;
  }
};

}  // namespace sfc::collections::hash

namespace sfc::collections {
using hash::HashMap;
}
