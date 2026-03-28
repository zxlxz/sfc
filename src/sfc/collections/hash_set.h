#pragma once

#include "sfc/collections/hash_tbl.h"

namespace sfc::collections::hash_set {

using hash_tbl::HashTbl;

template <class T>
class HashSet {
  struct Entry {
    T key;
  };
  HashTbl<Entry> _inn = {};

 public:
  static auto with_capacity(usize min_capacity) noexcept -> HashSet {
    auto res = HashSet{};
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
  auto contains(const auto& val) const noexcept -> bool {
    return _inn.search(val) != nullptr;
  }

  // Returns whether the value was newly inserted.
  auto insert(T val) noexcept -> bool {
    return _inn.try_insert({static_cast<T&&>(val)}) == nullptr;
  }

  // Returns whether the value was present in the set.
  auto remove(const auto& val) noexcept -> bool {
    return bool(_inn.remove(val));
  }

  void clear() {
    _inn.clear();
  }

 public:
  // trait: fmt::Display
  void fmt(auto& f) const {
    auto imp = f.debug_set();
    _inn.for_each([&](const Entry& entry) { imp.entry(entry.key); });
  }
};

}  // namespace sfc::collections::hash_set

namespace sfc::collections {
using hash_set::HashSet;
}
