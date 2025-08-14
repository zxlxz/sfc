#pragma once

#include "sfc/alloc.h"

namespace sfc::collections {

template <class T, class A = alloc::Global>
class HashTable {
  static constexpr u8 kEmpty = 0x80;
  static constexpr u8 kDeleted = 0xFF;
  static constexpr auto kAlign = 128U;
  static constexpr auto kMaxLoadFactor = 0.75;

  usize _mask = 0;
  u8* _ptr = nullptr;
  usize _rem = 0;
  usize _len = 0;

 public:
  constexpr HashTable() noexcept = default;

  ~HashTable() {
    this->clear();
    this->dealloc();
  }

  HashTable(HashTable&& other) noexcept
      : _mask{other._mask}, _ptr{other._ptr}, _rem{other._rem}, _len{other._len} {
    other._mask = 0;
    other._ptr = nullptr;
    other._rem = 0;
    other._len = 0;
  }

  HashTable& operator=(HashTable&& other) noexcept {
    if (this == &other) {
      return *this;
    }

    this->clear();
    this->dealloc();

    _mask = mem::take(other._mask);
    _ptr = mem::take(other._ptr);
    _rem = mem::take(other._rem);
    _len = mem::take(other._len);
    return *this;
  }

  auto data() const -> T* {
    const auto offs = (_mask + kAlign) & ~(kAlign - 1);
    return reinterpret_cast<T*>(_ptr + offs);
  }

  auto len() const -> usize {
    return _len;
  }

  auto capacity() const -> usize {
    return _mask == 0 ? 0 : _mask + 1;
  }

  auto search(const auto& key) const -> T* {
    if (_len == 0) {
      return nullptr;
    }

    const auto hx = this->hash(key);
    const auto h1 = hx & _mask;
    const auto h2 = static_cast<u8>(hx >> 57) & 0x7f;

    const auto offs = (_mask + kAlign) & ~(kAlign - 1);
    const auto data = reinterpret_cast<T*>(_ptr + offs);
    for (auto idx = h1;; idx = (idx + 1) & _mask) {
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
    const auto h1 = hx & _mask;
    const auto h2 = static_cast<u8>((hx >> 57) & 0x7F);

    const auto offs = (_mask + kAlign) & ~(kAlign - 1);
    const auto data = reinterpret_cast<T*>(_ptr + offs);
    auto del = _mask + 1;
    for (auto idx = h1;; idx = (idx + 1) & _mask) {
      if (_ptr[idx] == kEmpty) {
        const auto pos = del == _mask + 1 ? idx : del;
        this->insert_at(pos, h2, static_cast<T&&>(entry));
        break;
      } else if (_ptr[idx] == kDeleted) {
        (void)(del == _mask + 1 ? del = idx : del);
      } else if (_ptr[idx] == h2 && data[idx].key == entry.key) {
        return &data[idx];
      }
    }
    return nullptr;
  }

  auto erase_at(usize idx) -> bool {
    if (_len == 0 || idx > _mask || _ptr[idx] >= kEmpty) {
      return false;
    }

    const auto offs = (_mask + kAlign) & ~(kAlign - 1);
    const auto data = reinterpret_cast<T*>(_ptr + offs);

    _ptr[idx] = kDeleted;
    data[idx].~T();
    _len -= 1;
    return true;
  }

  void clear() {
    if (_ptr == nullptr || _len == 0) {
      return;
    }

    const auto off = (_mask + kAlign) & ~(kAlign - 1);
    const auto data = reinterpret_cast<T*>(_ptr + off);

    const auto cap = _mask + 1;
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

    const auto old_cap = _mask + 1;
    const auto old_offs = (old_cap - 1 + kAlign) & ~(kAlign - 1);
    const auto old_ctrl = _ptr;
    const auto old_data = reinterpret_cast<T*>(old_ctrl + old_offs);

    this->alloc(old_cap + additional);
    if (old_ctrl == nullptr) {
      return;
    }

    const auto new_offs = (_mask + kAlign) & ~(kAlign - 1);
    const auto new_data = reinterpret_cast<T*>(_ptr + new_offs);
    for (auto i = 0UL; i < old_cap; ++i) {
      if (old_ctrl[i] >= kEmpty) {
        continue;
      }
      auto& tmp = old_data[i];
      this->insert_new(tmp.key, static_cast<T&&>(tmp));
      tmp.~T();
    }

    A::dealloc(old_ctrl, {old_offs + old_cap * sizeof(T), kAlign});
  }

  void for_each(auto&& f) {
    if (_len == 0) {
      return;
    }

    const auto offs = (_mask + kAlign) & ~(kAlign - 1);
    const auto data = reinterpret_cast<T*>(_ptr + offs);
    for (auto i = 0UL; i < _mask + 1; ++i) {
      if (_ptr[i] < kEmpty) {
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

  void alloc(usize min_cap) {
    auto cap = 16U;
    while (cap < min_cap) {
      cap *= 2;
    }

    const auto offs = (cap - 1 + kAlign) & ~(kAlign - 1);
    const auto size = offs + (cap * sizeof(T));

    _mask = cap - 1;
    _ptr = static_cast<u8*>(A::alloc({size, kAlign}));
    _rem = static_cast<usize>(static_cast<f64>(cap) * kMaxLoadFactor);
    _len = 0;

    ptr::write_bytes(_ptr, kEmpty, cap);
  }

  void dealloc() {
    const auto offs = (_mask + kAlign) & ~(kAlign - 1);
    const auto size = offs + (_mask + 1) * sizeof(T);
    A::dealloc(_ptr, {size, kAlign});
  }

  auto insert_new(const auto& key, T&& val) -> bool {
    const auto hx = this->hash(key);
    const auto h1 = hx & _mask;
    const auto h2 = static_cast<u8>(hx >> 57) & 0x7f;

    for (auto idx = h1;; idx = (idx + 1) & _mask) {
      if (_ptr[idx] == kEmpty) {
        this->insert_at(idx, h2, static_cast<T&&>(val));
        return true;
      }
    }
    return false;
  }

  void insert_at(usize idx, usize h2, T&& val) {
    const auto offs = (_mask + kAlign) & ~(kAlign - 1);
    const auto data = reinterpret_cast<T*>(_ptr + offs);

    _ptr[idx] = h2;
    new (&data[idx]) T{static_cast<T&&>(val)};
    _len += 1;
    _rem -= 1;
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
    const auto p = _inn.search(key);
    if (!p) {
      return {};
    }
    auto res = Option<V>{static_cast<V&&>(p->val)};
    _inn.erase_at(static_cast<usize>(p - _inn.data()));
    return res;
  }

  void clear() {
    _inn.clear();
  }

 public:
  void fmt(auto& f) const {
    auto imp = f.debug_map();
    _inn.for_each([&](const auto& entry) { imp.entry(entry.key, entry.val); });
  }

  void serialize(auto& s) const {
    auto dict = s.new_dict();
    _inn.for_each([&](const auto& entry) { dict.insert(entry.key, s.ser(entry.val)); });
    return dict;
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

  ~HashSet() {}

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
    const auto p = _inn.search({static_cast<K&&>(val)});
    if (!p) {
      return false;
    }
    _inn.erase_at(static_cast<usize>(p - _inn.data()));
    return true;
  }
};

}  // namespace sfc::collections
