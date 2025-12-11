#pragma once

#include "sfc/collections/hash_tbl.h"

namespace sfc::collections::hash_set {

template <class K>
class HashSet {
  struct Entry {
    K key;
  };
  hash_tbl::HashTbl<Entry> _inn;

 public:
  HashSet() = default;

  ~HashSet() = default;

  HashSet(HashSet&& other) noexcept = default;

  HashSet& operator=(HashSet&& other) noexcept = default;

  static auto with_capacity(usize min_capacity) -> HashSet {
    auto res = HashSet{};
    res._inn.reserve(min_capacity);
    return res;
  }

  auto len() const -> usize {
    return _inn.len();
  }

  auto capacity() const -> usize {
    return _inn.capacity();
  }

  auto contains(const auto& val) const -> bool {
    const auto p = _inn.search({static_cast<K&&>(val)});
    return p != nullptr;
  }

  auto insert(K val) -> bool {
    const auto p = _inn.try_insert({static_cast<K&&>(val)});
    return p != nullptr;
  }

  auto remove(const auto& val) -> bool {
    return _inn.remove(val, [](auto&) {});
  }
};

}  // namespace sfc::collections::hash_set

namespace sfc::collections {
using hash_set::HashSet;
}
