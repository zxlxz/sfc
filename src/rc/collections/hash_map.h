#pragma once

#include "rc/core.h"

namespace rc::collections::hash_map {

template <class K, bool = is_int<K>()>
struct Hash;

template <class K, class V, bool = is_int<K>()>
class Pair;

template <class K>
struct Hash<K, true> {
 private:
  static auto _to_uint(const K& key) {
    if constexpr (__is_unsigned(K)) {
      return key;
    }
    if constexpr (rc::is_same<K, i8>()) {
      return u8(key);
    }
    if constexpr (rc::is_same<K, i16>()) {
      return u16(key);
    }
    if constexpr (rc::is_same<K, i32>()) {
      return u32(key);
    }
    if constexpr (rc::is_same<K, i64>()) {
      return u64(key);
    }
  }

 public:
  using H = decltype(_to_uint(K()));

  K _key;

  Hash(const K& key) noexcept : _key(key) {}

  auto hash() const noexcept -> H { return H(_key); }
};

template <class K>
struct Hash<K, false> {
  using H = decltype(static_cast<K*>(nullptr)->hash());

  H _idx;
  const K& _key;

  Hash(const K& key) noexcept : _idx(key.hash()), _key(key) {}

  auto hash() const noexcept -> H { return _idx; }
};

template <class K, class V>
class Pair<K, V, true> {
 public:
  using H = typename Hash<K>::H;

  K _key;
  V _val;

  template <class UK, class... UV>
  Pair(const H& idx, UK&& key, UV&&... vals)
      : _key(as_fwd<UK>(key)), _val(as_fwd<UV>(vals)...) {
    (void)idx;
  }

  auto hash() const noexcept -> H { return H(_key); }

  auto set_hash(const H& key) noexcept -> void { _key = H(key); }

  template <class UK>
  auto eq(const UK& key) const {
    return _key == key._key;
  }
};

template <class K, class V>
class Pair<K, V, false> {
 public:
  using H = typename Hash<K>::H;

  H _idx;
  K _key;
  V _val;

  template <class UK, class... UV>
  Pair(const H& idx, UK&& key, UV&&... vals)
      : _idx(idx), _key(as_fwd<UK>(key)), _val(as_fwd<UV>(vals)...) {}

  auto hash() const -> H { return _idx; }

  auto set_hash(const H& idx) noexcept -> void { _idx = idx; }

  template <class UK>
  auto eq(const UK& key) const {
    return _idx == key._idx && _key == key._key;
  }
};

template <class K, class V>
class HashMap {
 public:
  using Hash = Hash<K>;
  using Pair = Pair<K, V>;
  using Impl = HashTable<Pair>;
  Impl _impl;

  HashMap(usize capacity = 8) : _impl(capacity) {}

  auto len() const -> usize { return _impl.len(); }

  auto get(const K& key) -> const V& { return _impl.get(Hash(key))._val; }

  template <class UK, class... UT>
  auto insert(UK&& key, UT&&... val) -> void {
    auto hkey = Hash(key);
    auto addr = _impl._search_ins(hkey);
    ptr::ctor(&addr->_data, hkey.hash(), as_fwd<UK>(key), as_fwd<UT>(val)...);
  }
};

}  // namespace rc::collections::hash_map
