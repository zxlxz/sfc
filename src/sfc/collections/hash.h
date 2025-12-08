#pragma once

#include "sfc/alloc.h"

namespace sfc::collections {

template <class T>
class HashTable {
  using A = alloc::Global;

  static constexpr u8 kEmpty = 0x80;
  static constexpr u8 kDeleted = 0xFF;
  static constexpr auto kAlign = 128U;
  static constexpr auto kMaxLoadFactor = 0.75;

  u8* _ptr = nullptr;
  usize _msk = 0;
  usize _len = 0;
  usize _rem = 0;

 public:
  HashTable() noexcept = default;

  ~HashTable() {
    this->dealloc();
  }

  HashTable(HashTable&& other) noexcept : _ptr{other._ptr}, _msk{other._msk}, _len{other._len}, _rem{other._rem} {
    other._ptr = nullptr;
    other._msk = 0;
    other._len = 0;
    other._rem = 0;
  }

  HashTable& operator=(HashTable&& other) noexcept {
    if (this != &other) {
      this->dealloc();
      _ptr = mem::take(other._ptr);
      _msk = mem::take(other._msk);
      _rem = mem::take(other._rem);
      _len = mem::take(other._len);
    }
    return *this;
  }

  auto len() const -> usize {
    return _len;
  }

  auto capacity() const -> usize {
    return _msk == 0 ? 0 : _msk + 1;
  }

  auto search(const auto& key) const -> T* {
    if (_len == 0) {
      return nullptr;
    }

    const auto hx = this->hash(key);
    const auto h1 = hx & _msk;
    const auto h2 = static_cast<u8>(hx >> 57) & 0x7f;

    const auto data = this->data();
    for (auto idx = h1;; idx = (idx + 1) & _msk) {
      if (_ptr[idx] == kEmpty) {
        return nullptr;
      } else if (_ptr[idx] == h2 && data[idx].key == key) {
        return &data[idx];
      }
    }
    return nullptr;
  }

  auto search_or_insert(T&& entry) -> T* {
    if (_rem == 0) {
      this->reserve(1);
    }

    const auto hx = this->hash(entry.key);
    const auto h1 = hx & _msk;
    const auto h2 = static_cast<u8>((hx >> 57) & 0x7F);

    const auto data = this->data();
    auto del = _msk + 1;
    for (auto idx = h1;; idx = (idx + 1) & _msk) {
      if (_ptr[idx] == kEmpty) {
        const auto pos = del == _msk + 1 ? idx : del;
        this->insert_at(data, pos, h2, static_cast<T&&>(entry));
        break;
      } else if (_ptr[idx] == kDeleted) {
        (void)(del == _msk + 1 ? del = idx : del);
      } else if (_ptr[idx] == h2 && data[idx].key == entry.key) {
        return &data[idx];
      }
    }
    return nullptr;
  }

  auto remove(const auto& key, auto&& f) -> bool {
    if (_len == 0) {
      return false;
    }

    const auto hx = this->hash(key);
    const auto h1 = hx & _msk;
    const auto h2 = static_cast<u8>(hx >> 57) & 0x7f;

    const auto offs = (_msk + kAlign) & ~(kAlign - 1);
    const auto data = reinterpret_cast<T*>(_ptr + offs);
    for (auto idx = h1;; idx = (idx + 1) & _msk) {
      if (_ptr[idx] == kEmpty) {
        return false;
      } else if (_ptr[idx] == h2 && data[idx].key == key) {
        f(data[idx]);
        this->erase_at(data, idx);
        return true;
      }
    }
    return false;
  }

  void clear() {
    if (_ptr == nullptr || _len == 0) {
      return;
    }

    const auto off = (_msk + kAlign) & ~(kAlign - 1);
    const auto data = reinterpret_cast<T*>(_ptr + off);

    const auto cap = _msk + 1;
    for (auto i = 0UL; i < cap; ++i) {
      if (_ptr[i] < kEmpty) {
        data[i].~T();
      }
    }
    _len = 0;
    _rem = static_cast<usize>(static_cast<f64>(cap) * kMaxLoadFactor);
    ptr::write_bytes(_ptr, kEmpty, cap);
  }

  void reserve(usize additional) {
    if (additional < _rem) {
      return;
    }

    auto old_tbl = mem::move(*this);
    this->alloc(old_tbl.capacity() + additional);
    if (old_tbl._ptr == nullptr) {
      return;
    }

    const auto old_ctrl = old_tbl._ptr;
    const auto old_data = old_tbl.data();
    const auto new_data = this->data();
    for (auto i = 0UL; i < old_tbl._msk + 1; ++i) {
      if (old_ctrl[i] >= kEmpty) {
        continue;
      }
      auto& tmp = old_data[i];
      this->insert_new(new_data, tmp.key, static_cast<T&&>(tmp));
    }
  }

  void for_each(this auto& self, auto&& f) {
    if (self._len == 0) {
      return;
    }

    const auto ctrl = self._ptr;
    const auto data = self.data();
    for (auto i = 0UL; i < self._msk + 1; ++i) {
      if (ctrl[i] < kEmpty) {
        f(data[i]);
      }
    }
  }

 private:
  template <class K>
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

  auto data() const -> T* {
    const auto offs = (_msk + kAlign) & ~(kAlign - 1);
    return reinterpret_cast<T*>(_ptr + offs);
  }

  auto layout() const -> alloc::Layout {
    const auto cap = _msk + 1;
    const auto offs = (cap - 1 + kAlign) & ~(kAlign - 1);
    const auto size = offs + (cap * sizeof(T));
    return {size, kAlign};
  }

  void alloc(usize min_cap) {
    auto cap = 16U;
    while (cap < min_cap) {
      cap *= 2;
    }

    _msk = cap - 1;
    _ptr = static_cast<u8*>(A::alloc(this->layout()));
    _rem = static_cast<usize>(static_cast<f64>(cap) * kMaxLoadFactor);
    _len = 0;

    ptr::write_bytes(_ptr, kEmpty, cap);
  }

  void dealloc() {
    this->clear();
    A::dealloc(_ptr, this->layout());
  }

  auto insert_new(T* data, const auto& key, T&& val) -> bool {
    const auto hx = this->hash(key);
    const auto h1 = hx & _msk;
    const auto h2 = static_cast<u8>(hx >> 57) & 0x7f;

    for (auto idx = h1;; idx = (idx + 1) & _msk) {
      if (_ptr[idx] == kEmpty) {
        this->insert_at(data, idx, h2, static_cast<T&&>(val));
        return true;
      }
    }
    return false;
  }

  void insert_at(T* data, usize idx, u8 h2, T&& val) {
    _ptr[idx] = h2;
    new (&data[idx]) T{static_cast<T&&>(val)};
    _len += 1;
    _rem -= 1;
  }

  void erase_at(T* data, usize idx) {
    _ptr[idx] = kEmpty;
    data[idx].~T();
    _len -= 1;
  }
};

template <class K, class V>
class HashMap {
  struct Entry {
    K key;
    V val;
  };
  HashTable<Entry> _inn;

 public:
  HashMap() = default;

  ~HashMap() {}

  HashMap(HashMap&& other) noexcept = default;

  HashMap& operator=(HashMap&& other) noexcept = default;

  static auto with_capacity(usize min_capacity) -> HashMap {
    auto res = HashMap{};
    res._inn.reserve(min_capacity);
    return res;
  }

  auto len() const -> usize {
    return _inn.len();
  }

  auto capacity() const -> usize {
    return _inn.capacity();
  }

  auto get(const auto& key) const -> Option<const V&> {
    const auto ptr = _inn.search(key);
    if (!ptr) {
      return {};
    }
    return ptr->val;
  }

  auto get_mut(const auto& key) -> Option<V&> {
    const auto ptr = _inn.search(key);
    if (!ptr) {
      return {};
    }
    return ptr->val;
  }

  auto try_insert(K key, V val) -> Option<V&> {
    const auto p = _inn.search_or_insert({static_cast<K&&>(key), static_cast<V&&>(val)});
    if (!p) {
      return {};
    }
    return p->val;
  }

  auto insert(K key, V val) -> Option<V> {
    const auto p = _inn.search_or_insert({static_cast<K&&>(key), static_cast<V&&>(val)});
    if (!p) {
      return {};
    }
    return mem::replace(p->val, static_cast<V&&>(val));
  }

  auto remove(const K& key) -> Option<V> {
    auto res = Option<V>{};
    _inn.remove(key, [&res](auto& x) { res = static_cast<V&&>(x.val); });
    return res;
  }

  void clear() {
    _inn.clear();
  }

 public:
  // trait: fmt::Display
  void fmt(auto& f) const {
    auto imp = f.debug_map();
    _inn.for_each([&](const auto& entry) { imp.entry(entry.key, entry.val); });
  }

  // trait: serde::Serialize
  void serialize(auto& ser) const {
    auto map = ser.serialize_map();
    _inn.for_each([&](const auto& entry) { map.serialize_entry(entry.key, entry.val); });
    map.end();
  }

  // trait: serde::Deserialize
  template <class D, class E = typename D::Error>
  static auto deserialize(D& des) -> Result<HashMap, E> {
    auto res = HashMap{};
    auto visit = [&](auto& map) -> Result<void, E> {
      while (map.has_next()) {
        auto key = _TRY(map.template next_key<K>());
        auto val = _TRY(map.template next_value<V>());
        res.insert(key, static_cast<V&&>(val));
      }
      return {};
    };
    _TRY(des.deserialize_map(visit));
    return res;
  }
};

template <class K>
class HashSet {
  struct Entry {
    K key;
  };
  HashTable<Entry> _inn;

 public:
  HashSet() = default;

  ~HashSet() = default;

  HashSet(HashSet&& other) noexcept = default;

  HashSet& operator=(HashSet&& other) noexcept = default;

  static auto with_capacity(usize min_capacity) -> HashSet {
    auto res = HashSet{};
    res._inn.reserve(min_capacity);
    return res;
  }

  auto len() const -> usize {
    return _inn.len();
  }

  auto capacity() const -> usize {
    return _inn.capacity();
  }

  auto contains(const auto& val) const -> bool {
    const auto p = _inn.search({static_cast<K&&>(val)});
    return p != nullptr;
  }

  auto insert(K val) -> bool {
    const auto p = _inn.search_or_insert({static_cast<K&&>(val)});
    return p != nullptr;
  }

  auto remove(const auto& val) -> bool {
    return _inn.remove(val, [](auto&) {});
  }
};

}  // namespace sfc::collections
