#pragma once

#include "sfc/alloc/alloc.h"

namespace sfc::collections::hash {

static constexpr u8 CTRL_NUL = 0x80U;
static constexpr u8 CTRL_DEL = 0xFFU;

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
};

template <class T>
struct Bucket {
  static constexpr auto kInvalidIdx = num::Int<usize>::MAX;

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
      if (_ctrl[k] == CTRL_NUL) {
        return k;
      }
    }
    return kInvalidIdx;
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
    auto del_idx = kInvalidIdx;

    for (auto i = 0U; i <= _mask; ++i) {
      const auto k = (_hidx + i) & _mask;
      const auto c = _ctrl[k];
      if (c == h2 && _data[k].key == key) {
        return {&_data[k], k};
      } else if (c == CTRL_NUL) {
        const auto ins_idx = del_idx != kInvalidIdx ? del_idx : k;
        return {nullptr, ins_idx};
      } else if (c == CTRL_DEL && del_idx == kInvalidIdx) {
        del_idx = k;
      }
    }
    return {nullptr, kInvalidIdx};
  }

  void insert_at(usize pos, u8 h2, T&& val) {
    _ctrl[pos] = h2;
    ptr::write(_data + pos, mem::move(val));
  }

  auto erase_at(usize pos) -> T {
    auto res = ptr::read(_data + pos);
    _ctrl[pos] = CTRL_DEL;
    return res;
  }

  auto remove(u8 h2, const auto& key) -> Option<T> {
    const auto [ptr, idx] = this->search_key(h2, key);
    if (!ptr) return {};
    return this->erase_at(idx);
  }

  auto try_insert(u8 h2, T&& entry) -> Option<T&> {
    const auto [ptr, idx] = this->search_for_insert(h2, entry.key);
    if (ptr) {
      return *ptr;
    }

    this->insert_at(idx, h2, mem::move(entry));
    return {};
  }

  auto insert(u8 h2, T&& entry) -> Option<T> {
    const auto [ptr, idx] = this->search_for_insert(h2, entry.key);
    if (ptr) {
      return mem::replace(*ptr, mem::move(entry));
    }

    this->insert_at(idx, h2, mem::move(entry));
    return {};
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

template <class T, class A = alloc::Global>
class HashTbl {
  static constexpr f64 kLoadFactor = 0.75;
  u8* _ptr{nullptr};
  usize _cap{0};
  usize _len{0};
  usize _rem{0};
  [[no_unique_address]] A _a{};

 public:
  HashTbl() noexcept = default;

  ~HashTbl() noexcept {
    if (!_ptr) {
      return;
    }

    this->clear();
    _a.deallocate(_ptr, HashTbl::layout(_cap));
  }

  HashTbl(HashTbl&& other) noexcept
      : _ptr{mem::take(other._ptr)}
      , _cap{mem::take(other._cap)}
      , _len{mem::take(other._len)}
      , _rem{mem::take(other._rem)}
      , _a{mem::move(other._a)} {}

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

  static auto with_capacity(usize min_cap) -> HashTbl {
    static constexpr usize kMaxCap = num::Int<u32>::MAX;
    if (min_cap == 0) {
      return {};
    }

    sfc::assert_(min_cap <= kMaxCap, "HashTbl::with_capacity: min_cap(={}) > kMaxCap(={})", min_cap, kMaxCap);
    auto new_cap = usize{8U};
    while (new_cap < min_cap) {
      new_cap <<= 1;
    }

    auto res = HashTbl{};
    res._ptr = ptr::cast<u8>(res._a.allocate(HashTbl::layout(new_cap)));
    res._cap = new_cap;
    res.init();

    return res;
  }

  auto len() const noexcept -> usize {
    return _len;
  }

  auto cap() const noexcept -> usize {
    return _cap;
  }

  auto search(const auto& key) const -> T* {
    if (_len == 0) {
      return nullptr;
    }

    const auto [h1, h2] = this->hidx(key);
    return this->bucket(h1).search_key(h2, key).ptr;
  }

  auto try_insert(T&& entry) noexcept -> Option<T&> {
    this->reserve(1);

    const auto [h1, h2] = this->hidx(entry.key);
    auto bkt = this->bucket(h1);
    if (auto res = bkt.try_insert(h2, mem::move(entry))) {
      return res;
    }

    _len += 1;
    _rem -= 1;
    return {};
  }

  auto insert(T&& entry) noexcept -> Option<T> {
    this->reserve(1);

    const auto [h1, h2] = this->hidx(entry.key);
    auto bkt = this->bucket(h1);
    if (auto res = bkt.insert(h2, mem::move(entry))) {
      return res;
    }
    _len += 1;
    _rem -= 1;
    return {};
  }

  auto remove(const auto& key) noexcept -> Option<T> {
    if (_len == 0) {
      return {};
    }

    const auto [h1, h2] = this->hidx(key);
    auto ret = this->bucket(h1).remove(h2, key);
    if (ret) {
      _len -= 1;
    }
    return ret;
  }

  void clear() noexcept {
    if (_len == 0) {
      return;
    }
    this->iter_mut().for_each([&](T& entry) { entry.~T(); });
    this->init();
  }

  void reserve(usize additional) {
    if (additional <= _rem) {
      return;
    }

    this->rehash(_len + additional);
  }

  using Iter = hash::Iter<const T>;
  auto iter() const -> Iter {
    return {{}, this->ctrl(), this->data(), _cap};
  }

  using IterMut = hash::Iter<T>;
  auto iter_mut() -> IterMut {
    return {{}, this->ctrl(), this->data(), _cap};
  }

 private:
  static constexpr usize kAlign = 16U;

  static auto layout(usize cap) noexcept -> mem::Layout {
    const auto ctrl_size = __builtin_align_up(cap, kAlign);
    const auto data_size = cap * sizeof(T);
    return mem::Layout{ctrl_size + data_size, kAlign};
  }

  auto ctrl() const noexcept -> u8* {
    return _ptr;
  }

  auto data() const noexcept -> T* {
    const auto ctrl_size = __builtin_align_up(_cap, kAlign);
    return ptr::cast<T>(_ptr + ctrl_size);
  }

  void init() noexcept {
    if (_ptr == nullptr) {
      return;
    }
    const auto ctrl_size = __builtin_align_up(_cap, kAlign);
    ptr::write_bytes(_ptr, CTRL_NUL, ctrl_size);
    _len = 0;
    _rem = usize(f64(_cap) * kLoadFactor);
  }

  auto mask() const noexcept -> usize {
    return _cap - 1;
  }

  auto hidx(const auto& key) const noexcept -> Tuple<usize, u8> {
    const auto hx = Hash::hash(key);
    const auto h1 = hx & this->mask();
    const auto h2 = u8((hx >> 57) & 0x7F);
    return {h1, h2};
  }

  auto bucket(usize h1) const -> Bucket<T> {
    return Bucket{this->ctrl(), this->data(), this->mask(), h1};
  }

  void rehash(usize max_len) {
    const auto min_cap = usize(f64(max_len) / kLoadFactor + 0.5);
    auto new_tbl = HashTbl::with_capacity(min_cap);

    // rehash all entries
    this->iter_mut().for_each([&](T& entry) {
      const auto ret = new_tbl.rehash_insert(mem::move(entry));
      sfc::assert_(ret, "HashTbl::rehash failed");
    });
    *this = mem::move(new_tbl);
  }

  auto rehash_insert(T&& entry) -> bool {
    if (_rem == 0) {
      return false;
    }

    const auto [h1, h2] = this->hidx(entry.key);
    const auto ret = this->bucket(h1).insert_new(h2, mem::move(entry));
    if (ret) {
      _len += 1;
      _rem -= 1;
    }
    return ret;
  }
};

}  // namespace sfc::collections::hash
