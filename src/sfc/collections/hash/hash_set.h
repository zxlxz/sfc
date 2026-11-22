#pragma once

#include "sfc/collections/hash/hash_tbl.h"

namespace sfc::collections::hash {

template <class T, class A = alloc::Global>
class HashSet {
  struct Entry {
    T key;
  };
  HashTbl<Entry, A> _tbl;

  explicit HashSet(HashTbl<Entry, A> inn) noexcept : _tbl(mem::move(inn)) {}

 public:
  static auto new_(A alloc = {}) -> HashSet {
    return HashSet{HashTbl<Entry, A>::new_(alloc)};
  }

  static auto with_capacity(usize min_capacity, A alloc = {}) noexcept -> HashSet {
    return HashSet{HashTbl<Entry, A>::with_capacity(min_capacity, alloc)};
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
  auto contains(const auto& val) const noexcept -> bool {
    return _tbl.search(val) != nullptr;
  }

  // Returns whether the value was newly inserted.
  auto insert(T val) noexcept -> bool {
    const auto res = _tbl.try_insert({mem::move(val)});
    return !res;
  }

  // Returns whether the value was present in the set.
  auto remove(const auto& val) noexcept -> bool {
    return bool(_tbl.remove(val));
  }

  void clear() {
    _tbl.clear();
  }

 public:
  // trait: fmt::Display
  void fmt(auto& f) const {
    f.debug_set().entries(_tbl.iter()).finish();
  }
};

}  // namespace sfc::collections::hash

namespace sfc::collections {
using hash::HashSet;
}
