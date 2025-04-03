#pragma once

#include "sfc/alloc.h"

namespace sfc::collections {

template <class K, class V>
class [[nodiscard]] VecMap {
  Vec<K> _keys = {};
  Vec<V> _vals = {};

 public:
  VecMap() noexcept = default;

  ~VecMap() = default;

  VecMap(VecMap&&) noexcept = default;

  VecMap& operator=(VecMap&&) noexcept = default;

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
      auto res = mem::replace(*pos, static_cast<V&&>(val));
      return res;
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
  using Iter = iter::Zip<typename Vec<K>::Iter, typename Vec<V>::Iter>;
  auto iter() const -> Iter {
    return {_keys.iter(), _vals.iter()};
  }

  using IterMut = iter::Zip<typename Vec<K>::IterMut, typename Vec<V>::IterMut>;
  auto iter_mut() -> IterMut {
    return {_keys.iter_mut(), _vals.iter_mut()};
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
