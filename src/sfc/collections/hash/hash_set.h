#pragma once

#include "sfc/collections/hash/hash_tbl.h"

namespace sfc::collections::hash {

template <class T, class A = alloc::Global>
class HashSet {
  struct Entry {
    T key;
  };
  HashTbl<Entry, A> _inn;

  explicit HashSet(HashTbl<Entry, A> inn) noexcept : _inn(mem::move(inn)) {}

 public:
  HashSet(HashSet&&) noexcept = default;
  ~HashSet() = default;

  HashSet& operator=(HashSet&&) noexcept = default;

  static auto new_(A alloc = {}) -> HashSet {
    return HashSet{HashTbl<Entry, A>::new_(alloc)};
  }

  static auto with_capacity(usize min_capacity, A alloc = {}) noexcept -> HashSet {
    return HashSet{HashTbl<Entry, A>::with_capacity(min_capacity, alloc)};
  }

 public:
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
    const auto res = _inn.try_insert({mem::move(val)});
    return !res;
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
    f.debug_set().entries(_inn.iter()).finish();
  }
};

}  // namespace sfc::collections::hash

namespace sfc::collections {
using hash::HashSet;
}
