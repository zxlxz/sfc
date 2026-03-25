#pragma once

#include "sfc/alloc/vec.h"

namespace sfc::collections::hash {

static constexpr u32 CTRL_ALIGN = 128UL;
static constexpr u8 CTRL_NUL = 0x80U;
static constexpr u8 CTRL_DEL = 0xFFU;

struct Hasher {
  static auto hash(const auto& key) noexcept -> u64 {
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
  usize _mask;
  usize _idx = 0;

 public:
  auto next() -> Option<T&> {
    while (_idx <= _mask) {
      const auto ctrl = _ctrl[_idx++];
      if (ctrl < CTRL_NUL) {
        return _data[_idx - 1];
      }
    }
    return Option<T&>{};
  }
};

template <class T>
class HashTbl {
  u8* _ptr = nullptr;
  usize _cap = 0;
  usize _len = 0;
  usize _remain = 0;

 public:
  HashTbl() noexcept = default;

  ~HashTbl() noexcept {
    if (!_ptr) {
      return;
    }

    this->clear();
    __builtin_operator_delete(_ptr);
  }

  HashTbl(HashTbl&& other) noexcept
      : _ptr{mem::take(other._ptr)}
      , _cap{mem::take(other._cap)}
      , _len{mem::take(other._len)}
      , _remain{mem::take(other._remain)} {}

  HashTbl& operator=(HashTbl&& other) noexcept {
    if (this != &other) {
      mem::swap(_ptr, other._ptr);
      mem::swap(_cap, other._cap);
      mem::swap(_len, other._len);
      mem::swap(_remain, other._remain);
    }
    return *this;
  }

  auto len() const noexcept -> usize {
    return _len;
  }

  auto capacity() const noexcept -> usize {
    return _cap;
  }

  auto search(const auto& key) const -> const T* {
    if (_len == 0) {
      return nullptr;
    }

    const auto [h1, h2] = this->hidx(key);
    const auto bucket = this->bucket(h1);
    return bucket.search_key(h2, key);
  }

  auto try_insert(T&& entry) noexcept -> T* {
    this->reserve(1);

    const auto [h1, h2] = this->hidx(entry.key);
    auto bucket = this->bucket(h1);
    const auto [ptr, idx] = bucket.search_key_or_nil(h2, entry.key);
    if (!ptr && idx != static_cast<usize>(-1)) {
      bucket.insert_at(idx, h2, static_cast<T&&>(entry));
      _len += 1;
      _remain -= 1;
    }
    return ptr;
  }

  auto try_erase(T* dst) noexcept -> bool {
    const auto data = this->data();
    const auto idx = static_cast<usize>(dst - data);
    if (idx >= _cap) {  // check if dst belongs to this table
      return false;
    }

    // mark as deleted
    data[idx].~T();
    _ptr[idx] = CTRL_DEL;
    _len -= 1;
    return true;
  }

  void reserve(usize additional) {
    if (additional < _remain) {
      return;
    }

    // alloc new
    const auto min_cap = (_len + additional) * 4 / 3;
    const auto new_cap = num::next_power_of_two<usize>(min_cap);
    this->realloc(cmp::max<usize>(new_cap, 8U));
  }

  void clear() noexcept {
    if (_len == 0) {
      return;
    }
    this->iter_mut().for_each([](T& entry) { entry.~T(); });
    ptr::write_bytes(_ptr, CTRL_NUL, _cap);
    _len = 0;
    _remain = _cap * 3 / 4;
  }

  using Iter = hash::Iter<const T>;
  auto iter() const -> Iter {
    return Iter{{}, _ptr, this->data(), _cap - 1};
  }

  using IterMut = hash::Iter<T>;
  auto iter_mut() -> IterMut {
    return IterMut{{}, _ptr, this->data(), _cap - 1};
  }

 private:
  struct HIdx {
    u64 h1;
    u8 h2;
  };

  struct Bucket {
    u8* _ctrl;
    T* _data;
    usize _mask;
    usize _h1;

   public:
    auto search_nul() const -> usize {
      for (auto idx = _h1;;) {
        const auto ctrl = _ctrl[idx];
        if (ctrl == CTRL_NUL) {
          return idx;
        }
        if ((idx = (idx + 1) & _mask) == _h1) {
          break;
        }
      }
      return static_cast<usize>(-1);
    }

    auto search_key(u8 h2, const auto& key) const -> T* {
      for (auto idx = _h1;;) {
        const auto ctrl = _ctrl[idx];
        if (ctrl == h2 && _data[idx].key == key) {  // found
          return &_data[idx];
        } else if (ctrl == CTRL_NUL) {  // not found
          return nullptr;
        }
        if ((idx = (idx + 1) & _mask) == _h1) {
          return nullptr;
        }
      }
    }

    struct KeyOrNullResult {
      T* ptr = nullptr;
      usize idx = 0;
    };

    auto search_key_or_nil(u8 h2, const auto& key) const -> KeyOrNullResult {
      auto del_idx = Option<usize>{};

      for (auto idx = _h1;;) {
        const auto ctrl = _ctrl[idx];
        if (ctrl == h2 && _data[idx].key == key) {
          return {&_data[idx]};
        } else if (ctrl == CTRL_NUL) {
          return {nullptr, del_idx.unwrap_or(idx)};
        } else if (ctrl == CTRL_DEL && del_idx.is_none()) {
          del_idx = idx;
        }
        if ((idx = (idx + 1) & _mask) == _h1) {
          return {nullptr};
        }
      }
    }

    void insert_at(usize pos, u8 h2, T&& val) {
      _ctrl[pos] = h2;
      ptr::write(_data + pos, static_cast<T&&>(val));
    }
  };

  auto bucket(usize h1) const -> Bucket {
    const auto offset = num::align_up(_cap, CTRL_ALIGN);
    const auto data = reinterpret_cast<T*>(_ptr + offset);
    return {_ptr, data, _cap - 1, h1};
  }

  auto hidx(const auto& key) const noexcept -> HIdx {
    const auto hx = Hasher::hash(key);
    const auto h1 = hx & (_cap - 1);
    const auto h2 = static_cast<u8>((hx >> 57) & 0x7F);
    return {h1, h2};
  }

  auto data() const noexcept -> T* {
    const auto offset = num::align_up(_cap, CTRL_ALIGN);
    return reinterpret_cast<T*>(_ptr + offset);
  }

  auto realloc(usize new_cap) -> bool {
    if (new_cap < _len) {
      return false;
    }

    // alloc new memory
    const auto ctrl_size = num::align_up(new_cap, CTRL_ALIGN);
    const auto data_size = new_cap * sizeof(T);
    const auto new_ptr = static_cast<u8*>(__builtin_operator_new(ctrl_size + data_size));
    ptr::write_bytes(new_ptr, CTRL_NUL, ctrl_size);

    // save old state
    auto old_iter = this->iter_mut();
    const auto old_ptr = mem::replace(_ptr, new_ptr);
    const auto old_cap = mem::replace(_cap, new_cap);
    const auto old_len = mem::replace(_len, 0);
    _remain = _cap * 3 / 4;

    // old->new
    if (old_len != 0) {
      this->rehash(old_iter);
    }

    if (old_ptr) {
      __builtin_operator_delete(old_ptr);
    }

    return true;
  }

  void rehash(IterMut iter) noexcept {
    while (auto opt = iter.next()) {
      if (_remain == 0) {
        break;
      }
      auto& val = *opt;
      const auto [h1, h2] = this->hidx(val.key);

      auto bucket = this->bucket(h1);
      const auto ins_pos = bucket.search_nul();
      bucket.insert_at(ins_pos, h2, static_cast<T&&>(val));
      _len += 1;
      _remain -= 1;
    }
  }
};

}  // namespace sfc::collections::hash
