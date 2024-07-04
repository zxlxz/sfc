#pragma once

#include "sfc/alloc.h"

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
      : _len{mem::take(other._len)}
      , _mask{mem::take(other._mask)}
      , _ctrl{mem::take(other._ctrl)}
      , _keys{mem::take(other._keys)}
      , _vals{mem::take(other._vals)} {}

  HashMap& operator=(HashMap&& other) noexcept {
    if (this == &other) {
      return *this;
    }
    _len = mem::take(other._len);
    _mask = mem::take(other._mask);
    _ctrl = mem::move(other._ctrl);
    _keys = mem::move(other._keys);
    _vals = mem::move(other._vals);
    return *this;
  }

  static auto with_capacity(usize min_capacity) -> HashMap {
    if (min_capacity == 0) {
      return {};
    }

    auto res = HashMap{};
    res.init(min_capacity);
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
    const auto [idx, inserted] = this->find_or_insert(mem::move(key), mem::move(val));
    if (!inserted) {
      return {};
    }
    return _vals[idx];
  }

  auto insert(K key, V val) -> Option<V> {
    const auto [idx, inserted] = this->find_or_insert(mem::move(key), mem::move(val));
    if (!inserted) {
      return {};
    }

    auto res = Option<V>{mem::move(_vals[idx])};
    _vals[idx] = mem::move(val);
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

    for (auto i = 0UL; i < _ctrl._cap; ++i) {
      if (_ctrl[i] < kDelete) {
        _keys[i].~K();
        _vals[i].~V();
      }
    }
    __builtin_memset(_ctrl._ptr, kEmpty, _ctrl._cap);

    _len = 0;
  }

 public:
  void fmt(auto& f) const {
    auto imp = f.debug_map();
    for (auto i = 0UL; i < _ctrl._cap; ++i) {
      if (_ctrl[i] < kDelete) {
        imp.entry(_keys[i], _vals[i]);
      }
    }
  }

  void serialize(auto& s) const {
    auto dict = s.new_dict();
    for (auto i = 0UL; i < _ctrl._cap; ++i) {
      if (_ctrl[i] < kDelete) {
        dict.insert(_keys[i], s.ser(_vals[i]));
      }
    }
    return dict;
  }

 private:
  static auto hash(const K& key) -> usize {
    if constexpr (requires { key.hash(); }) {
      return key.hash();
    } else {
      const auto* p = static_cast<const u8*>(static_cast<const void*>(&key));
      auto h = usize{0xcbf29ce484222325ULL};
      for (auto i = 0U; i < sizeof(K); ++i) {
        h ^= p[i];
        h *= 0x100000001b3ULL;
      }
      return h;
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
    if (min_capacity == 0) {
      _len = 0;
      _mask = 0;
      return;
    }

    auto new_cap = usize{8U};
    while (new_cap < min_capacity) {
      new_cap *= 2;
    }

    _len = 0;
    _mask = new_cap - 1;
    _ctrl.reserve_extract(0, new_cap);
    _keys.reserve_extract(0, new_cap);
    _vals.reserve_extract(0, new_cap);
    ptr::write_bytes(_ctrl._ptr, kEmpty, _ctrl._cap);
  }

  void insert_at(usize idx, u8 ctrl, K&& key, V&& val) {
    _ctrl[idx] = ctrl;
    new (&_keys[idx]) K{mem::move(key)};
    new (&_vals[idx]) V{mem::move(val)};
    _len += 1;
  }

  auto erase_at(usize idx) -> V {
    auto res = mem::move(_vals[idx]);
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

  struct FindOrInsert {
    usize idx;
    bool inserted;
  };
  auto find_or_insert(K&& key, V&& val) -> FindOrInsert {
    this->reserve(1);

    const auto [h1, h2] = this->hidx(key);
    for (auto idx = h1;; idx = (idx + 1) & _mask) {
      const auto c = _ctrl[idx];
      if (c >= kDelete) {
        this->insert_at(idx, h2, mem::move(key), mem::move(val));
        return {idx, true};
      }

      if (c == h2 && key == _keys[idx]) {
        return {idx, false};
      }
    }

    return {static_cast<usize>(-1), false};
  }

  void reserve(usize additional) {
    if (_len + additional <= _ctrl._cap * 3 / 4) {
      return;
    }

    auto olen = mem::take(_len);
    auto ctrl = mem::move(_ctrl);
    auto keys = mem::move(_keys);
    auto vals = mem::move(_vals);
    this->init(olen + additional);

    for (auto i = 0UL; _len < olen; ++i) {
      if (ctrl[i] >= kDelete) {
        continue;
      }

      auto& k = keys[i];
      auto& v = vals[i];
      this->try_insert(mem::move(k), mem::move(v));
      k.~K();
      v.~V();
    }
  }
};

}  // namespace sfc::collections
