#pragma once

#include "sfc/alloc.h"
#include "sfc/core.h"

namespace sfc::collections {

template <class K, class V, class A = alloc::Global>
class HashMap {
  static constexpr usize kMinCap = 16U;
  static constexpr u8 kDelete = 0x80U;
  static constexpr u8 kEmpty = 0xFFU;

  struct HIdx {
    usize idx;
    u8 ctrl;
  };

  usize _len = 0;
  usize _mask = 0;
  u8* _ctrl = nullptr;
  K* _keys = nullptr;
  V* _vals = nullptr;
  A _alloc = {};

 public:
  HashMap() = default;

  ~HashMap() {
    this->drop();
  }

  HashMap(HashMap&& other) noexcept
      : _len{mem::take(other._len)},
        _mask{mem::take(other._mask)},
        _ctrl{mem::take(other._ctrl)},
        _keys{mem::take(other._keys)},
        _vals{mem::take(other._vals)},
        _alloc{mem::move(other._alloc)} {}

  HashMap& operator=(HashMap&& other) noexcept {
    if (this != &other) {
      this->drop();
      _len = mem::take(other._len);
      _mask = mem::take(other._mask);
      _ctrl = mem::take(other._ctrl);
      _keys = mem::take(other._keys);
      _vals = mem::take(other._vals);
      _alloc = mem::move(other._alloc);
    }
    return *this;
  }

  static auto with_capacity(usize capacity) -> HashMap {
    if (capacity == 0) {
      return {};
    }

    auto res = HashMap{};
    res.init(capacity);
    return res;
  }

  auto len() const -> usize {
    return _len;
  }

  auto capacity() const -> usize {
    return _ctrl ? _mask + 1 : 0U;
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

  auto insert(K key, V val) -> Option<V&> {
    const auto idx = this->try_insert(static_cast<K&&>(key), static_cast<V&&>(val));
    if (idx != -1) {
      return _vals[idx];
    }

    return {};
  }

  auto replace(K key, V val) -> Option<V> {
    const auto idx = this->try_insert(static_cast<K&&>(key), static_cast<V&&>(val));
    if (idx != -1) {
      auto res = Option<V>{static_cast<V&&>(_vals[idx])};
      _vals[idx] = static_cast<V&&>(val);
      return res;
    }

    return {};
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
    __builtin_memset(_ctrl, kEmpty, _mask + 1);

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

  void init(usize min_capacity) {
    if (_ctrl || min_capacity == 0) {
      return;
    }

    auto capacity = kMinCap;
    while (capacity < min_capacity) {
      capacity *= 2;
    }

    _mask = capacity - 1;
    _ctrl = _alloc.template alloc_array<u8>(capacity);
    _keys = _alloc.template alloc_array<K>(capacity);
    _vals = _alloc.template alloc_array<V>(capacity);
    __builtin_memset(_ctrl, kEmpty, capacity);
  }

  void drop() {
    if (!_ctrl) {
      return;
    }

    this->clear();
    _alloc.dealloc_array(_ctrl, _mask + 1);
    _alloc.dealloc_array(_keys, _mask + 1);
    _alloc.dealloc_array(_vals, _mask + 1);
    _ctrl = nullptr;
    _keys = nullptr;
    _vals = nullptr;
  }

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

  auto try_insert(K&& key, V&& val) -> isize {
    if (_len * 4 >= _mask * 3) {
      this->rehash();
    }

    const auto [h1, h2] = this->hidx(key);
    for (auto idx = h1;; idx = (idx + 1) & _mask) {
      const auto c = _ctrl[idx];
      if (c >= kDelete) {
        this->insert_at(idx, h2, static_cast<K&&>(key), static_cast<V&&>(val));
        return -1;
      }

      if (c == h2 && key == _keys[idx]) {
        return static_cast<isize>(idx);
      }
    }

    return -1;
  }

  void rehash() {
    const auto old_cap = this->capacity();
    const auto new_cap = old_cap == 0 ? kMinCap : old_cap * 2;
    auto src = static_cast<HashMap&&>(*this);
    this->init(new_cap);

    for (auto i = 0U; i < old_cap; ++i) {
      const auto c = src._ctrl[i];
      if (c >= kDelete) {
        continue;
      }
      auto& k = src._keys[i];
      auto& v = src._vals[i];
      this->insert(static_cast<K&&>(k), static_cast<V&&>(v));
      k.~K();
      v.~V();
    }
  }
};

}  // namespace sfc::collections
