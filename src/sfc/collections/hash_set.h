#pragma once

#include "sfc/collections/hash_tbl.h"

namespace sfc::collections {

template <class K>
class HashSet {
  struct Entry {
    K key;
  };
  HashTbl<Entry> _inn = {};

 public:
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

}  // namespace sfc::collections
