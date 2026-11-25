#pragma once

#include "sfc/collections/str_set.h"

namespace sfc::collections {

template <class T>
class StrMap {
  StrSet _keys;
  List<T> _vals;

  StrMap(StrSet keys, List<T> vals) noexcept : _keys{mem::move(keys)}, _vals{mem::move(vals)} {}

 public:
  static auto new_() -> StrMap {
    return StrMap{StrSet::new_(), List<T>::new_()};
  }

  static auto with_capacity(usize capacity) -> StrMap {
    return StrMap{StrSet::with_capacity(capacity), List<T>::with_capacity(capacity)};
  }

 public:
  auto capacity() const -> usize {
    return _vals.capacity();
  }

  auto len() const -> usize {
    return _vals.len();
  }

  auto get(Str key) const -> Option<const T&> {
    const auto uid = _keys.search(key);
    if (!uid) {
      return None{};
    }
    return _vals[*uid];
  }

  auto get_mut(Str key) -> Option<T&> {
    const auto uid = _keys.search(key);
    if (!uid) {
      return None{};
    }
    return _vals[*uid];
  }

  auto try_insert(Str key, T val) -> Option<T> {
    const auto [uid, inserted] = _keys.insert(key);
    if (!inserted) {  // not inserted, key already exists
      return mem::move(val);
    }

    _vals.reserve(1);
    _vals.push(mem::move(val));
    return None{};
  }

  auto insert(Str key, T val) -> Option<T> {
    const auto [uid, inserted] = _keys.insert(key);
    if (!inserted) {  // not inserted, key already exists
      auto& dst = _vals[uid];
      return mem::replace(dst, mem::move(val));
    }

    _vals.reserve(1);
    _vals.push(mem::move(val));
    return None{};
  }

  void reserve(usize add_cnt) {
    _keys.reserve(add_cnt);
    _vals.reserve(add_cnt);
  }
};

}  // namespace sfc::collections
