#pragma once

#include "sfc/collections/hash_tbl.h"

namespace sfc::collections {

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
    return _inn.capacity();
  }

  void reserve(usize additional) {
    _inn.reserve(additional);
  }

 public:
  auto contains(const auto& val) const noexcept -> bool {
    const auto p = _inn.search(val);
    return p != nullptr;
  }

  // Returns whether the value was newly inserted.
  auto insert(T val) noexcept -> bool {
    const auto p = _inn.try_insert({static_cast<T&&>(val)});
    return p == nullptr;
  }

  // Returns whether the value was present in the set.
  auto remove(const auto& val) noexcept -> bool {
    if (auto p = const_cast<Entry*>(_inn.search(val))) {
      (void)_inn.try_erase(p);
      return true;
    }
    return false;
  }

  void clear() {
    _inn.clear();
  }

 public:
  // trait: fmt::Display
  void fmt(auto& f) const {
    auto imp = f.debug_set();
    _inn.for_each([&](const Entry& entry) { imp.entry(entry.key); });
    imp.finish();
  }
};

}  // namespace sfc::collections
