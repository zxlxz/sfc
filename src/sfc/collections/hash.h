#pragma once

#include "sfc/alloc.h"
#include "sfc/core.h"

namespace sfc::collections {

template <class K, class V>
class HashMap {
  static constexpr u8 kDelete = 0x80U;
  static constexpr u8 kEmpty = 0xFFU;

  struct HIdx {
    usize idx;
    u8 ctrl;
  };

  usize _len{0};
  usize _mask{0};
  vec::Buf<u8> _ctrl{};
  vec::Buf<K> _keys{};
  vec::Buf<V> _vals{};

 public:
  HashMap() = default;

  ~HashMap() {}

  HashMap(HashMap&& other) noexcept
      : _len{mem::take(other._len)},
        _mask{mem::take(other._mask)},
        _ctrl{mem::take(other._ctrl)},
        _keys{mem::take(other._keys)},
        _vals{mem::take(other._vals)} {}

  HashMap& operator=(HashMap&& other) noexcept {
    if (this != &other) {
      _len = mem::take(other._len);
      _mask = mem::take(other._mask);
      _ctrl = mem::move(other._ctrl);
      _keys = mem::move(other._keys);
      _vals = mem::move(other._vals);
    }
    return *this;
  }

  static auto with_capacity(usize min_capacity) -> HashMap {
    if (min_capacity == 0) {
      return {};
    }

    auto cap = 1U;
    while (cap < min_capacity) {
      cap *= 2;
    }

    auto res = HashMap{};
    res._mask = cap - 1;
    res._ctrl.reserve_extract(0, cap);
    res._keys.reserve_extract(0, cap);
    res._vals.reserve_extract(0, cap);
    __builtin_memset(res._ctrl._ptr, kEmpty, cap);
    return res;
  }

  auto len() const -> usize {
    return _len;
  }

  auto capacity() const -> usize {
    return _ctrl._cap;
  }

  auto get(const auto& key) const -> Option<const V&> {
    if (_len == 0) {
      return {};
    }

    const auto idx = this->find(key);
    if (idx == -1) {
      return {};
    }
    return _vals[idx];
  }

  auto get_mut(const auto& key) -> Option<V&> {
    if (_len == 0) {
      return {};
    }

    const auto idx = this->find(key);
    if (idx == -1) {
      return {};
    }
    return _vals[idx];
  }

  auto try_insert(K key, V val) -> Option<V&> {
    const auto idx = this->find_or_insert(static_cast<K&&>(key), static_cast<V&&>(val));
    if (idx == -1) {
      return {};
    }
    return _vals._ptr[idx];
  }

  auto insert(K key, V val) -> Option<V> {
    const auto idx = this->find_or_insert(static_cast<K&&>(key), static_cast<V&&>(val));
    if (idx == -1) {
      return {};
    }

    auto res = Option<V>{static_cast<V&&>(_vals._ptr[idx])};
    _vals._ptr[idx] = static_cast<V&&>(val);
    return res;
  }

  auto remove(const K& key) -> Option<V> {
    if (_len == 0) {
      return {};
    }

    const auto idx = this->find(key);
    if (idx == -1) {
      return {};
    }

    return this->erase_at(static_cast<usize>(idx));
  }

  void clear() {
    if (_len == 0) {
      return;
    }

    for (auto i = 0UL; i <= _mask; ++i) {
      if (_ctrl[i] < kDelete) {
        _keys[i].~K();
        _vals[i].~V();
      }
    }
    __builtin_memset(_ctrl._ptr, kEmpty, _mask + 1);

    _len = 0;
  }

 private:
  static auto hash(const K& key) -> usize {
    if constexpr (requires { key.hash(); }) {
      return key.hash();
    } else {
      return static_cast<usize>(key);
    }
  }

  auto hidx(const K& key) const -> HIdx {
    static constexpr u8 kMask = 0x7Fu;

    const auto h0 = HashMap::hash(key);
    const auto h1 = h0 & _mask;
    const auto h2 = (h0 >> 57) & kMask;
    return {h1, static_cast<u8>(h2)};
  };

  auto insert_at(usize idx, u8 ctrl, K&& key, V&& val) {
    _ctrl[idx] = ctrl;
    new (&_keys[idx]) K{static_cast<K&&>(key)};
    new (&_vals[idx]) V{static_cast<V&&>(val)};
    _len += 1;
  }

  auto erase_at(usize idx) -> V {
    auto res = static_cast<V&&>(_vals[idx]);
    _keys[idx].~K();
    _vals[idx].~V();
    _ctrl[idx] = kDelete;
    _len -= 1;
    return res;
  }

  auto find(const auto& key) const -> isize {
    if (_len == 0) {
      return -1;
    }

    const auto [h1, h2] = this->hidx(key);
    for (auto idx = h1;; idx = (idx + 1) & _mask) {
      const auto c = _ctrl[idx];
      if (c == h2 && key == _keys[idx]) {
        return static_cast<isize>(idx);
      }
      if (c == kEmpty) {
        return -1;
      }
    }
  }

  auto find_or_insert(K&& key, V&& val) -> isize {
    if (_len * 4 >= _mask * 3) {
      this->rehash();
    }

    const auto [h1, h2] = this->hidx(key);
    for (auto idx = h1;; idx = (idx + 1) & _mask) {
      const auto c = _ctrl[idx];
      if (c >= kDelete) {
        this->insert_at(idx, h2, static_cast<K&&>(key), static_cast<V&&>(val));
        break;
      }

      if (c == h2 && key == _keys[idx]) {
        return static_cast<isize>(idx);
      }
    }

    return -1;
  }

  void rehash() {
    constexpr auto kMinCap = 4U;

    const auto old_cap = this->capacity();
    const auto new_cap = old_cap < kMinCap ? kMinCap * 2 : old_cap * 2;

    auto tmp = HashMap::with_capacity(new_cap);

    for (auto i = 0U; tmp._len < _len; ++i) {
      const auto c = _ctrl._ptr[i];
      if (c >= kDelete) {
        continue;
      }

      auto& k = _keys._ptr[i];
      auto& v = _vals._ptr[i];
      tmp.try_insert(static_cast<K&&>(k), static_cast<V&&>(v));
      k.~K();
      v.~V();
    }

    *this = static_cast<HashMap&&>(tmp);
  }
};

}  // namespace sfc::collections
