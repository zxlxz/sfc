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

class HashTblStorage {
  using A = alloc::Global;
  static constexpr usize kAlign = 16U;

  u8* _ptr{nullptr};
  usize _cap{0};
  [[no_unique_address]] A _alloc{};

 public:
  HashTblStorage();
  ~HashTblStorage();

  HashTblStorage(HashTblStorage&& other) noexcept;
  HashTblStorage& operator=(HashTblStorage&& other) noexcept;

  static auto with_capacity(usize min_cap, usize element_size) -> HashTblStorage;

  void init(usize element_size);

 public:
  auto cap() const noexcept -> usize {
    return _cap;
  }

  auto mask() const noexcept -> usize {
    return _cap - 1;
  }

  auto ctrl() const noexcept -> u8* {
    return _ptr;
  }

  template <class T>
  auto data() const noexcept -> T* {
    const auto offset = num::align_up(_cap, kAlign);
    return ptr::cast<T>(_ptr + offset);
  }
};

template <class T>
class HashTbl {
  using RawTbl = HashTblStorage;
  static constexpr f64 kLoadFactor = 0.75;

  RawTbl _buf;
  usize _len{0};
  usize _rem{0};

 public:
  HashTbl() noexcept = default;

  ~HashTbl() noexcept {
    this->clear();
  }

  HashTbl(HashTbl&& other) noexcept : _buf{mem::move(other._buf)}, _len{other._len}, _rem{other._rem} {
    other._len = 0;
    other._rem = 0;
  }

  HashTbl& operator=(HashTbl&& other) noexcept {
    if (this == &other) return *this;
    mem::swap(_buf, other._buf);
    mem::swap(_len, other._len);
    mem::swap(_rem, other._rem);
    return *this;
  }

  static auto with_capacity(usize min_cap) -> HashTbl {
    auto res = HashTbl{};
    res._buf = RawTbl::with_capacity(min_cap, sizeof(T));
    res.init();
    return res;
  }

  auto len() const noexcept -> usize {
    return _len;
  }

  auto cap() const noexcept -> usize {
    return _buf.cap();
  }

  auto search(const auto& key) const -> T* {
    if (_len == 0) {
      return nullptr;
    }

    const auto [h1, h2] = this->hidx(key);
    return this->bucket(h1).search_key(h2, key).ptr;
  }

  auto try_insert(T&& entry) noexcept -> T* {
    this->reserve(1);

    const auto [h1, h2] = this->hidx(entry.key);
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

    const auto [h1, h2] = this->hidx(key);
    auto ret = this->bucket(h1).remove(h2, key);
    if (ret) {
      _len -= 1;
    }
    return ret;
  }

  void clear() noexcept {
    if (_buf.cap() == 0) {
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
    return {{}, _buf.ctrl(), _buf.data<T>(), _buf.cap()};
  }

  using IterMut = hash::Iter<T>;
  auto iter_mut() -> IterMut {
    return {{}, _buf.ctrl(), _buf.data<T>(), _buf.cap()};
  }

 private:
  auto hidx(const auto& key) const noexcept -> Tuple<usize, u8> {
    const auto hx = Hash::hash(key);
    const auto h1 = hx & (_buf.mask());
    const auto h2 = u8((hx >> 57) & 0x7F);
    return {h1, h2};
  }

  auto bucket(usize h1) const -> Bucket<T> {
    return Bucket{_buf.ctrl(), _buf.data<T>(), _buf.mask(), h1};
  }

  void init() {
    _len = 0;
    _rem = usize(f64(_buf.cap()) * kLoadFactor);
    _buf.init(sizeof(T));
  }

  void rehash(usize max_len) {
    const auto min_cap = usize(f64(max_len) / kLoadFactor + 0.5);
    auto new_tbl = HashTbl::with_capacity(min_cap);

    // rehash all entries
    this->iter_mut().for_each([&](T& entry) { new_tbl.rehash_insert(mem::move(entry)); });
    *this = mem::move(new_tbl);
  }

  auto rehash_insert(T&& entry) -> bool {
    if (_rem == 0) {
      return false;
    }

    const auto [h1, h2] = this->hidx(entry.key);
    const auto ret = this->bucket(h1).insert_new(h2, mem::move(entry));
    if (!ret) {
      return false;
    }

    _len += 1;
    _rem -= 1;
    return true;
  }
};

}  // namespace sfc::collections::hash
