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
    panicking::assert_fmt(idx, "VecMap::[]: key(=`{}`) not exists", key);
    return *idx;
  }

  auto operator[](const auto& key) -> V& {
    const auto opt = this->get_mut(key);
    panicking::assert_fmt(opt, "VecMap::[]: key(=`{}`) not exists", key);
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

  void reserve(usize additional) {
    _keys.reserve(additional);
    _vals.reserve(additional);
  }

 public:
  auto iter() const -> iter::Zip<slice::Iter<const K>, slice::Iter<const V>> {
    return iter::Zip{{}, _keys.iter(), _vals.iter()};
  }

  auto iter_mut() -> iter::Zip<slice::Iter<K>, slice::Iter<V>> {
    return iter::Zip{{}, _keys.iter_mut(), _vals.iter_mut()};
  }

  auto contains_key(const auto& key) const -> bool {
    const auto val = this->get(key);
    return static_cast<bool>(val);
  }

  void fmt(auto& f) const {
    f.debug_map().entries(this->iter());
  }
};

}  // namespace sfc::collections

namespace sfc {
using collections::VecMap;
}  // namespace sfc
