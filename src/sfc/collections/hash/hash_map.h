#pragma once

#include "sfc/collections/hash/hash_tbl.h"

namespace sfc::collections::hash {

template <class K, class V, class A = alloc::Global>
class HashMap {
  struct Entry {
    K key;
    V val;
  };
  HashTbl<Entry, A> _tbl;

  explicit HashMap(HashTbl<Entry, A> inn) : _tbl(mem::move(inn)) {}

 public:
  static auto new_(A alloc = {}) -> HashMap {
    return HashMap{HashTbl<Entry, A>::new_(alloc)};
  }

  static auto with_capacity(usize min_capacity, A alloc = {}) -> HashMap {
    return HashMap{HashTbl<Entry, A>::with_capacity(min_capacity, alloc)};
  }

 public:
  auto len() const noexcept -> usize {
    return _tbl.len();
  }

  auto capacity() const noexcept -> usize {
    return _tbl.cap();
  }

  void reserve(usize additional) {
    _tbl.reserve(additional);
  }

 public:
  auto contains_key(const auto& key) const noexcept -> bool {
    const auto p = _tbl.search(key);
    return p != nullptr;
  }

  auto get(const auto& key) const noexcept -> Option<const V&> {
    if (auto* p = _tbl.search(key)) {
      return p->val;
    }
    return {};
  }

  auto get_mut(const auto& key) noexcept -> Option<V&> {
    if (auto* p = _tbl.search(key)) {
      return p->val;
    }
    return {};
  }

  auto try_insert(K key, V val) noexcept -> Option<V&> {
    auto tmp = Entry{mem::move(key), mem::move(val)};
    if (auto ref = _tbl.try_insert(mem::move(tmp))) {
      return ref->val;
    }
    return {};
  }

  auto insert(K key, V val) noexcept -> Option<V> {
    auto tmp = Entry{mem::move(key), mem::move(val)};
    if (auto res = _tbl.insert(mem::move(tmp))) {
      return mem::move(res->val);
    }
    return {};
  }

  auto remove(const K& key) -> Option<V> {
    return _tbl.remove(key).map([](auto entry) { return mem::move(entry.val); });
  }

  void clear() {
    _tbl.clear();
  }

 public:
  // trait: fmt::Display
  void fmt(auto& f) const {
    f.debug_map().entries(_tbl.iter()).finish();
  }

  // trait: serde::Serialize
  void serialize(auto& ser) const {
    auto imp = ser.serialize_obj();
    _tbl.iter().for_each([&](const Entry& entry) { imp.serialize_entry(entry.key, entry.val); });
  }

  // trait: serde::Deserialize
  template <class D, class E = typename D::Error>
  static auto deserialize(D& des) -> Result<HashMap, E> {
    auto res = HashMap::new_();

    _TRY(des.as_map().template for_each<V>([&](auto key, auto val) {
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
