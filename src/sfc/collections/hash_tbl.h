#pragma once

#include "sfc/alloc/vec.h"

namespace sfc::collections::hash_tbl {

static constexpr u32 CTRL_ALIGN = 128UL;
static constexpr u8 CTRL_NUL = 0x80U;
static constexpr u8 CTRL_DEL = 0xFFU;

struct Hasher {
  [[gnu::always_inline]] static auto hash(const auto& key) noexcept -> u64 {
    if constexpr (requires { key.hash(); }) {
      return key.hash();
    } else if constexpr (requires { static_cast<u64>(key); }) {
      return static_cast<u64>(key);
    } else {
      static_assert(false, "HashTbl::hash: cannot hash key type");
    }
  }
};

template <class T>
struct Iter : iter::Iterator<T&> {
  const u8* _ctrl;
  T* _data;
  usize _cap = 0;
  usize _idx = 0;

 public:
  auto next() -> Option<T&> {
    for (; _idx < _cap; _idx++) {
      if (_ctrl[_idx] < CTRL_NUL) {
        return _data[_idx++];
      }
    }
    return {};
  }

  void for_each(auto&& f) {
    for (; _idx < _cap; _idx++) {
      if (_ctrl[_idx] < CTRL_NUL) {
        f(_data[_idx]);
      }
    }
  }
};

template <class T>
struct Bucket {
  u8* _ctrl;
  T* _data;
  usize _mask;
  usize _hidx;

 public:
  struct SearchResult {
    T* ptr;
    usize idx;
  };

  auto search_nul() const -> usize {
    for (auto i = 0U; i <= _mask; ++i) {
      const auto k = (_hidx + i) & _mask;
      const auto c = _ctrl[k];
      if (c == CTRL_NUL) {
        return k;
      }
    }
    return static_cast<usize>(-1);
  }

  auto search_key(u8 h2, const auto& key) const -> SearchResult {
    for (auto i = 0U; i <= _mask; ++i) {
      const auto k = (_hidx + i) & _mask;
      const auto c = _ctrl[k];
      if (c == h2 && _data[k].key == key) {  // found
        return {&_data[k], k};
      } else if (c == CTRL_NUL) {  // not found
        return {nullptr, 0};
      }
    }
    return {nullptr, 0};
  }

  auto search_for_insert(u8 h2, const auto& key) const -> SearchResult {
    static constexpr auto INVALID_IDX = static_cast<usize>(-1);
    auto del_idx = INVALID_IDX;

    for (auto i = 0U; i <= _mask; ++i) {
      const auto k = (_hidx + i) & _mask;
      const auto c = _ctrl[k];
      if (c == h2 && _data[k].key == key) {
        return {&_data[k], k};
      } else if (c == CTRL_NUL) {
        const auto ins_idx = del_idx != INVALID_IDX ? del_idx : k;
        return {nullptr, ins_idx};
      } else if (c == CTRL_DEL && del_idx == INVALID_IDX) {
        del_idx = k;
      }
    }
    return {nullptr, INVALID_IDX};
  }

  void insert_at(usize pos, u8 h2, T&& val) {
    _ctrl[pos] = h2;
    ptr::write(_data + pos, static_cast<T&&>(val));
  }

  auto erase_at(usize pos) -> T {
    auto res = static_cast<T&&>(_data[pos]);
    _data[pos].~T();
    _ctrl[pos] = CTRL_DEL;
    return res;
  }

  auto remove(u8 h2, const auto& key) -> Option<T> {
    const auto [ptr, idx] = this->search_key(h2, key);
    if (!ptr) return {};
    return this->erase_at(idx);
  }

  auto try_insert(u8 h2, T&& entry) -> T* {
    const auto [ptr, idx] = this->search_for_insert(h2, entry.key);
    if (!ptr) {
      this->insert_at(idx, h2, static_cast<T&&>(entry));
      return nullptr;
    }
    return ptr;
  }

  auto insert_new(u8 h2, T&& entry) -> bool {
    const auto idx = this->search_nul();
    if (idx == static_cast<usize>(-1)) {
      return false;
    }
    this->insert_at(idx, h2, static_cast<T&&>(entry));
    return true;
  }
};

template <class T, class A = alloc::Global>
class HashTbl {
  static constexpr auto kLoadFactor = 4U;
  struct HIdx {
    u64 h1;
    u8 h2;
  };

  u8* _ptr{nullptr};
  usize _cap{0};
  usize _len{0};
  usize _rem{0};
  [[no_unique_address]] A _a{};

 public:
  HashTbl() noexcept = default;

  ~HashTbl() noexcept {
    if (_ptr == nullptr) {
      return;
    }

    this->clear();

    const auto layout = alloc::Layout{_cap + _cap * sizeof(T), alignof(T)};
    _a.dealloc(_ptr, layout);
  }

  HashTbl(HashTbl&& other) noexcept
      : _ptr{other._ptr}, _cap{other._cap}, _len{other._len}, _rem{other._rem}, _a{other._a} {
    other._ptr = nullptr;
    other._cap = 0;
    other._len = 0;
    other._rem = 0;
  }

  HashTbl& operator=(HashTbl&& other) noexcept {
    if (this != &other) {
      mem::swap(_ptr, other._ptr);
      mem::swap(_cap, other._cap);
      mem::swap(_len, other._len);
      mem::swap(_rem, other._rem);
      mem::swap(_a, other._a);
    }
    return *this;
  }

  static auto with_capacity(usize min_cap, A a = {}) -> HashTbl {
    const auto cap = num::next_power_of_two(min_cap);
    const auto layout = alloc::Layout{cap + cap * sizeof(T), alignof(T)};

    auto res = HashTbl{};
    res._ptr = static_cast<u8*>(a.alloc(layout));
    res._cap = cap;
    res.clear();
    return res;
  }

  auto len() const noexcept -> usize {
    return _len;
  }

  auto cap() const noexcept -> usize {
    return _cap;
  }

  void swap(HashTbl& other) noexcept {
    if (this != &other) {
      mem::swap(_ptr, other._ptr);
      mem::swap(_cap, other._cap);
      mem::swap(_len, other._len);
      mem::swap(_rem, other._rem);
      mem::swap(_a, other._a);
    }
  }

  auto hash(const auto& key) const noexcept -> HIdx {
    const auto hx = Hasher::hash(key);
    const auto h1 = hx & (_cap - 1);
    const auto h2 = static_cast<u8>((hx >> 57) & 0x7F);
    return {h1, h2};
  }

  auto search(const auto& key) const -> T* {
    if (_len == 0) {
      return nullptr;
    }

    const auto [h1, h2] = this->hash(key);
    return this->bucket(h1).search_key(h2, key).ptr;
  }

  auto try_insert(T&& entry) noexcept -> T* {
    this->reserve(1);

    const auto [h1, h2] = this->hash(entry.key);
    const auto ptr = this->bucket(h1).try_insert(h2, static_cast<T&&>(entry));
    if (!ptr) {
      _len += 1;
      _rem -= 1;
    }
    return ptr;
  }

  auto remove(const auto& key) noexcept -> Option<T> {
    if (_len == 0) {
      return {};
    }

    const auto [h1, h2] = this->hash(key);
    auto ret = this->bucket(h1).remove(h2, key);
    if (ret) {
      _len -= 1;
    }
    return ret;
  }

  void clear() noexcept {
    this->iter_mut().for_each([&](T& entry) { entry.~T(); });
    _len = 0;

    _rem = _cap - _cap / kLoadFactor;
    ptr::write_bytes(_ptr, CTRL_NUL, _cap);
  }

  void reserve(usize additional) {
    if (additional <= _rem) {
      return;
    }

    this->rehash(_len + additional);
  }

  void rehash(u32 min_cap) {
    if (min_cap < _len) return;

    // caculate new capacity
    auto new_cap = usize{1};
    while (new_cap - new_cap / kLoadFactor < min_cap) {
      new_cap *= 2;
    }

    auto new_tbl = HashTbl::with_capacity(new_cap, _a);
    this->iter_mut().for_each([&](T& entry) {
      const auto [h1, h2] = new_tbl.hash(entry.key);
      new_tbl.bucket(h1).insert_new(h2, static_cast<T&&>(entry));
      new_tbl._len += 1;
      new_tbl._rem -= 1;
    });

    this->swap(new_tbl);
  }

  auto bucket(usize h1) const -> Bucket<T> {
    const auto ctrl = _ptr;
    const auto data = reinterpret_cast<T*>(_ptr + _cap);
    const auto mask = _cap - 1;
    return Bucket{ctrl, data, mask, h1};
  }

  using Iter = hash_tbl::Iter<const T>;
  auto iter() const -> Iter {
    const auto ctrl = _ptr;
    const auto data = reinterpret_cast<T*>(_ptr + _cap);
    return {{}, ctrl, data, _cap};
  }

  using IterMut = hash_tbl::Iter<T>;
  auto iter_mut() -> IterMut {
    const auto ctrl = _ptr;
    const auto data = reinterpret_cast<T*>(_ptr + _cap);
    return {{}, ctrl, data, _cap};
  }
};

}  // namespace sfc::collections::hash_tbl
