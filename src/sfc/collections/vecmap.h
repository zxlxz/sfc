#pragma once

#include "sfc/alloc.h"

namespace sfc::collections {

template <class K, class V>
class VecMap {
  Vec<K> _keys = {};
  Vec<V> _vals = {};

 public:
  VecMap() noexcept = default;

  ~VecMap() = default;

  VecMap(VecMap&&) noexcept = default;

  auto operator=(VecMap&&) noexcept -> VecMap& = default;

  auto len() const -> usize {
    return _keys.len();
  }

  auto capacity() const -> usize {
    return _keys.capacity();
  }

  auto is_empty() const -> bool {
    return _keys.is_empty();
  }

  auto keys() const -> Slice<const K> {
    return _keys.as_slice();
  }

  auto vals() const -> Slice<const V> {
    return _vals.as_slice();
  }

  auto vals_mut() -> Slice<V> {
    return _vals.as_mut_slice();
  }

  auto get(const auto& key) const -> Option<const V&> {
    const auto idx = _keys.iter().position([&](const K& t) { return t == key; });
    if (!idx) {
      return {};
    }
    return _vals[*idx];
  }

  auto get_mut(const auto& key) -> Option<V&> {
    const auto idx = _keys.iter().position([&](const K& t) { return t == key; });
    if (!idx) {
      return {};
    }
    return _vals[*idx];
  }

  auto operator[](const auto& key) const -> const V& {
    const auto idx = this->get(key);
    panicking::assert(idx, "VecMap::[]: key(=`{}`) not exists", key);
    return *idx;
  }

  auto operator[](const auto& key) -> V& {
    const auto opt = this->get_mut(key);
    panicking::assert(opt, "VecMap::[]: key(=`{}`) not exists", key);
    return *opt;
  }

  auto insert(K key, V val) -> Option<V> {
    auto pos = this->get_mut(key);
    if (pos) {
      return Option{mem::replace(*pos, static_cast<V&&>(val))};
    }
    _keys.push(static_cast<K&&>(key));
    _vals.push(static_cast<V&&>(val));
    return {};
  }

  auto try_insert(K key, V val) -> Option<V&> {
    auto pos = this->get_mut(key);
    if (pos) {
      return pos;
    }
    _keys.push(static_cast<K&&>(key));
    _vals.push(static_cast<V&&>(val));
    return {};
  }

  void clear() {
    _keys.clear();
    _vals.clear();
  }

  void reserve(usize amt) {
    _keys.reserve(amt);
    _vals.reserve(amt);
  }

 public:
  auto contains_key(const auto& key) const -> bool {
    return bool(this->get(key));
  }

  void fmt(auto& f) const {
    const auto cnt = _keys.len();

    auto imp = f.debug_map();
    for (auto i = 0U; i < cnt; ++i) {
      imp.entry(_keys[i], _vals[i]);
    }
  }

  auto serialize(auto& s) const {
    auto dict = s.new_dict();
    for (auto i = 0U; i < _keys.len(); ++i) {
      dict.insert(_keys[i], s.ser(_vals[i]));
    }
    return dict;
  }
};

}  // namespace sfc::collections
