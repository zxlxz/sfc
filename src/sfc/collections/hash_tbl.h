#pragma once

#include "sfc/alloc/vec.h"

namespace sfc::collections::hash_tbl {

static constexpr auto CTRL_ALIGN = 16UL;
static constexpr auto CTRL_NUL = u8{0x80U};
static constexpr auto CTRL_DEL = u8{0xFFU};

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
  static constexpr auto kMaxSize = num::max_value<usize>() >> 8U;
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

    _a.dealloc(_ptr, this->layout());
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
    const auto ctrl_size = num::align_up(cap, CTRL_ALIGN);
    const auto data_size = cap * sizeof(T);
    const auto layout = alloc::Layout{ctrl_size + data_size, alignof(T)};

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

    const auto ctrl_size = num::align_up(_cap, CTRL_ALIGN);
    ptr::write_bytes(_ptr, CTRL_NUL, ctrl_size);
  }

  void reserve(usize additional) {
    if (additional <= _rem) {
      return;
    }

    this->rehash(_len + additional);
  }

  void rehash(usize min_cap) {
    if (min_cap < _len) {
      return;
    }
    sfc::expect(min_cap < kMaxSize, "HashTbl::rehash: requested capacity(={}) too large", min_cap);

    // calculate new capacity
    auto new_cap = num::next_power_of_two(min_cap);
    if (new_cap - new_cap / kLoadFactor < min_cap) {
      new_cap *= 2;
    }

    auto tmp = mem::replace(*this, HashTbl::with_capacity(new_cap, _a));
    tmp.iter_mut().for_each([&](T& entry) { this->rehash_insert(static_cast<T&&>(entry)); });
  }

  using Iter = hash_tbl::Iter<const T>;
  auto iter() const -> Iter {
    const auto ctrl_size = num::align_up(_cap, CTRL_ALIGN);
    const auto data = reinterpret_cast<const T*>(_ptr + ctrl_size);
    return {{}, _ptr, data, _cap};
  }

  using IterMut = hash_tbl::Iter<T>;
  auto iter_mut() -> IterMut {
    const auto ctrl_size = num::align_up(_cap, CTRL_ALIGN);
    const auto data = reinterpret_cast<T*>(_ptr + ctrl_size);
    return {{}, _ptr, data, _cap};
  }

 private:
  [[gnu::always_inline]] auto layout() const -> alloc::Layout {
    const auto ctrl_size = num::align_up(_cap, CTRL_ALIGN);
    const auto data_size = _cap * sizeof(T);
    return alloc::Layout{ctrl_size + data_size, alignof(T)};
  }

  [[gnu::always_inline]] auto bucket(usize h1) const -> Bucket<T> {
    const auto ctrl_size = num::align_up(_cap, CTRL_ALIGN);
    const auto data = reinterpret_cast<T*>(_ptr + ctrl_size);
    return Bucket{_ptr, data, _cap - 1, h1};
  }

  auto rehash_insert(T&& entry) -> bool {
    if (_rem == 0) {
      return false;
    }

    const auto [h1, h2] = this->hash(entry.key);
    this->bucket(h1).insert_new(h2, static_cast<T&&>(entry));
    _len += 1;
    _rem -= 1;
    return true;
  }
};

}  // namespace sfc::collections::hash_tbl
