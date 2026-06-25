#pragma once

#include "sfc/alloc/alloc.h"

namespace sfc::collections::hash {

struct Hasher {
  template <class T>
  static auto hash(const T& key) noexcept -> u64 {
    if constexpr (requires { key.hash(); }) {
      return key.hash();
    } else if constexpr (trait::uint_<T>) {
      return u64{key};
    } else if constexpr (trait::sint_<T>) {
      return num::cast_unsigned(key);
    } else {
      static_assert(false, "HashTbl::hash: cannot hash key type");
    }
  }
};

struct Ctrl {
  static constexpr auto kNul = u8{0x80U};
  static constexpr auto kDel = u8{0xFFU};
  u8 _val;

 public:
  void set_nul() noexcept {
    _val = kNul;
  }

  void set_del() noexcept {
    _val = kDel;
  }

  auto is_nul() const noexcept -> bool {
    return _val == kNul;
  }

  auto is_del() const noexcept -> bool {
    return _val == kDel;
  }

  auto is_valid() const noexcept -> bool {
    return _val < kNul;
  }
};

template <class T>
struct Iter : iter::Iterator {
  using Item = T&;

  const Ctrl* _ctrl;
  T* _data;
  usize _cap = 0;
  usize _idx = 0;

 public:
  auto next() -> Option<Item> {
    for (; _idx < _cap; _idx++) {
      if (_ctrl[_idx].is_valid()) {
        return _data[_idx++];
      }
    }
    return {};
  }
};

template <class T>
struct Bucket {
  static constexpr auto kInvalidIdx = num::Int<usize>::MAX;

  Ctrl* _ctrl;
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
      if (_ctrl[k].is_nul()) {
        return k;
      }
    }
    return kInvalidIdx;
  }

  auto search_key(u8 h2, const auto& key) const -> SearchResult {
    for (auto i = 0U; i <= _mask; ++i) {
      const auto k = (_hidx + i) & _mask;
      const auto c = _ctrl[k];
      if (c._val == h2 && _data[k].key == key) {  // found
        return {&_data[k], k};
      } else if (c.is_nul()) {  // not found
        return {nullptr, 0};
      }
    }
    return {nullptr, 0};
  }

  auto search_for_insert(u8 h2, const auto& key) const -> SearchResult {
    auto del_idx = kInvalidIdx;

    for (auto i = 0U; i <= _mask; ++i) {
      const auto k = (_hidx + i) & _mask;
      const auto c = _ctrl[k];
      if (c._val == h2 && _data[k].key == key) {
        return {&_data[k], k};
      } else if (c.is_nul()) {
        const auto ins_idx = del_idx != kInvalidIdx ? del_idx : k;
        return {nullptr, ins_idx};
      } else if (c.is_del() && del_idx == kInvalidIdx) {
        del_idx = k;
      }
    }
    return {nullptr, kInvalidIdx};
  }

  void insert_at(usize pos, u8 h2, T&& val) {
    _ctrl[pos] = {h2};
    ptr::write(_data + pos, mem::move(val));
  }

  auto erase_at(usize pos) -> T {
    auto res = ptr::read(_data + pos);
    _ctrl[pos].set_del();
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
      this->insert_at(idx, h2, mem::move(entry));
      return nullptr;
    }
    return ptr;
  }

  auto insert_new(u8 h2, T&& entry) -> bool {
    const auto idx = this->search_nul();
    if (idx == kInvalidIdx) {
      return false;
    }
    this->insert_at(idx, h2, mem::move(entry));
    return true;
  }
};

template <class T>
class HashTbl {
  static constexpr auto kMaxSize = num::Int<u32>::MAX;
  static constexpr auto kLoadFactor = 4U;
  static constexpr auto CTRL_ALIGN = usize{128UL};

  struct HIdx {
    u64 h1;
    u8 h2;
  };

  u8* _ptr{nullptr};
  usize _cap{0};
  usize _len{0};
  usize _rem{0};
  Allocator _a{alloc::global()};

 public:
  HashTbl() noexcept = default;

  ~HashTbl() noexcept {
    if (_ptr == nullptr) return;

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
    if (this == &other) return *this;
    this->swap(other);
    return *this;
  }

  static auto with_capacity(usize min_cap, Allocator alloc = alloc::global()) -> HashTbl {
    if (min_cap == 0) {
      return HashTbl{};
    }

    auto res = HashTbl{};
    res._cap = num::next_power_of_two(min_cap);
    res._ptr = ptr::cast<u8>(alloc.alloc(res.layout()));
    res._a = alloc;
    res.init();
    return res;
  }

  auto len() const noexcept -> usize {
    return _len;
  }

  auto cap() const noexcept -> usize {
    return _cap;
  }

  void swap(HashTbl& other) noexcept {
    if (this == &other) return;
    mem::swap(_ptr, other._ptr);
    mem::swap(_cap, other._cap);
    mem::swap(_len, other._len);
    mem::swap(_rem, other._rem);
    mem::swap(_a, other._a);
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
    const auto ptr = this->bucket(h1).try_insert(h2, mem::move(entry));
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
    this->init();
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
    sfc::assert_(min_cap < kMaxSize, "HashTbl::rehash: requested capacity(={}) too large", min_cap);

    // calculate new capacity
    auto new_cap = num::next_power_of_two(min_cap);
    if (new_cap - new_cap / kLoadFactor < min_cap) {
      new_cap *= 2;
    }

    auto tmp = mem::replace(*this, HashTbl::with_capacity(new_cap, _a));
    tmp.iter_mut().for_each([&](T& entry) { this->rehash_insert(mem::move(entry)); });
  }

  using Iter = hash::Iter<const T>;
  auto iter() const -> Iter {
    const auto ctrl_size = num::align_up(_cap, CTRL_ALIGN);
    const auto ctrl = ptr::cast<const Ctrl>(_ptr);
    const auto data = ptr::cast<T>(_ptr + ctrl_size);
    return {{}, ctrl, data, _cap};
  }

  using IterMut = hash::Iter<T>;
  auto iter_mut() -> IterMut {
    const auto ctrl_size = num::align_up(_cap, CTRL_ALIGN);
    const auto ctrl = ptr::cast<Ctrl>(_ptr);
    const auto data = ptr::cast<T>(_ptr + ctrl_size);
    return {{}, ctrl, data, _cap};
  }

 private:
  auto layout() const -> mem::Layout {
    const auto ctrl_size = num::align_up(_cap, CTRL_ALIGN);
    const auto data_size = _cap * sizeof(T);
    return mem::Layout{ctrl_size + data_size, alignof(T)};
  }

  auto bucket(usize h1) const -> Bucket<T> {
    const auto ctrl_size = num::align_up(_cap, CTRL_ALIGN);
    const auto ctrl = ptr::cast<Ctrl>(_ptr);
    const auto data = ptr::cast<T>(_ptr + ctrl_size);
    return Bucket{ctrl, data, _cap - 1, h1};
  }

  void init() {
    const auto ctrl_size = num::align_up(_cap, CTRL_ALIGN);
    const auto ctrl = ptr::cast<Ctrl>(_ptr);
    ptr::write_bytes(ctrl, Ctrl::kNul, ctrl_size);
    _len = 0;
    _rem = _cap - _cap / kLoadFactor;
  }

  auto rehash_insert(T&& entry) -> bool {
    if (_rem == 0) {
      return false;
    }

    const auto [h1, h2] = this->hash(entry.key);
    this->bucket(h1).insert_new(h2, mem::move(entry));
    _len += 1;
    _rem -= 1;
    return true;
  }
};

}  // namespace sfc::collections::hash
