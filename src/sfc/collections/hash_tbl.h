#pragma once

#include "sfc/alloc/vec.h"

namespace sfc::collections {

template <class T>
class HashTbl {
  static constexpr u8 CTRL_NUL = 0x80U;
  static constexpr u8 CTRL_DEL = 0xFFU;

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
    auto ctrl_ptr = _ptr;
    const auto data_ptr = this->data();
    const auto mask = _cap - 1;

    const auto [h1, h2] = this->hidx(key);
    for (auto i = 0UL; i < _cap; ++i) {
      const auto idx = (h1 + i) & mask;
      const auto ctrl = ctrl_ptr[idx];
      const auto& item = data_ptr[idx];
      if (ctrl == h2 && item.key == key) {  // found
        return &item;
      } else if (ctrl == CTRL_NUL) {  // not found
        break;
      }
    }
    return nullptr;
  }

  auto try_insert(T&& entry) noexcept -> T* {
    this->reserve(1);

    const auto ctrl_ptr = _ptr;
    const auto data_ptr = this->data();

    auto insert_at = [&](usize idx, usize ctrl, T& entry) {
      ctrl_ptr[idx] = ctrl;
      ptr::write(data_ptr + idx, static_cast<T&&>(entry));
      _len += 1;
      _remain -= 1;
    };

    const auto [h1, h2] = this->hidx(entry.key);
    auto del_idx = _cap;
    for (auto i = 0UL; i < _cap; ++i) {
      const auto idx = (h1 + i) & (_cap - 1);
      const auto ctrl = ctrl_ptr[idx];
      const auto& item = data_ptr[idx];
      if (ctrl == h2 && item.key == entry.key) {
        return data_ptr + idx;
      } else if (ctrl == CTRL_NUL) {
        insert_at(del_idx == _cap ? idx : del_idx, h2, entry);
        return nullptr;
      } else if (ctrl == CTRL_DEL && del_idx == _cap) {
        del_idx = idx;
      }
    }
    return nullptr;
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
    const auto new_cap = num::next_power_of_two<usize>(min_cap, 8);
    this->realloc(new_cap);
  }

  void clear() noexcept {
    if (_len == 0) {
      return;
    }
    this->for_each(mem::drop<T>);
    ptr::write_bytes(_ptr, CTRL_NUL, _cap);
    _len = 0;
    _remain = _cap * 3 / 4;
  }

  void for_each(this auto& self, auto&& f) {
    if (self._len == 0) {
      return;
    }
    const auto ctrl_ptr = self._ptr;
    const auto data_ptr = self.data();
    for (usize idx = 0; idx < self._cap; ++idx) {
      if (ctrl_ptr[idx] < CTRL_NUL) {
        f(data_ptr[idx]);
      }
    }
  }

 private:
  static constexpr auto CTRL_ALIGN = 128UL;

  struct HIdx {
    u64 h1;
    u8 h2;
  };

  static auto hash(const auto& key) noexcept -> u64 {
    if constexpr (requires { key.hash(); }) {
      return key.hash();
    } else if constexpr (requires { static_cast<u64>(key); }) {
      return static_cast<u64>(key);
    } else {
      static_assert(false, "HashTbl::hash: cannot hash key type");
    }
  }

  auto hidx(const auto& key) const noexcept -> HIdx {
    const auto hx = HashTbl::hash(key);
    const auto h1 = hx & (_cap - 1);
    const auto h2 = static_cast<u8>((hx >> 57) & 0x7F);
    return {h1, h2};
  }

  auto size() const noexcept -> usize {
    const auto ctrl_size = (_cap + CTRL_ALIGN - 1) & ~(CTRL_ALIGN - 1);
    const auto data_size = _cap * sizeof(T);
    return ctrl_size + data_size;
  }

  auto data() const noexcept -> T* {
    const auto ctrl_size = (_cap + CTRL_ALIGN - 1) & ~(CTRL_ALIGN - 1);
    return reinterpret_cast<T*>(_ptr + ctrl_size);
  }

  void rehash(u8* old_ctrl, T* old_data, usize old_cap) noexcept {
    const auto new_ctrl = _ptr;
    const auto new_data = this->data();

    auto insert_new = [&](T&& entry) {
      const auto [h1, h2] = this->hidx(entry.key);
      for (auto idx = h1;; idx = (idx + 1) & (_cap - 1)) {
        if (new_ctrl[idx] == CTRL_NUL) {
          new_ctrl[idx] = h2;
          ptr::write(new_data + idx, static_cast<T&&>(entry));
          _len += 1;
          _remain -= 1;
          break;
        }
      }
    };

    for (usize idx = 0; idx < old_cap; ++idx) {
      const auto ctrl = old_ctrl[idx];
      if (ctrl < CTRL_NUL) {
        auto entry = ptr::read(old_data + idx);
        insert_new(static_cast<T&&>(entry));
      }
    }
  }

  void realloc(usize new_cap) {
    // save old state
    const auto old_ptr = _ptr;
    const auto old_cap = _cap;
    const auto old_len = _len;
    const auto old_data = this->data();

    // init new state
    _cap = new_cap;
    _ptr = static_cast<u8*>(__builtin_operator_new(this->size()));
    _len = 0;
    _remain = _cap * 3 / 4;
    ptr::write_bytes(_ptr, CTRL_NUL, _cap);

    // old->new
    if (old_len != 0) {
      this->rehash(old_ptr, old_data, old_cap);
    }

    if (old_ptr) {
      __builtin_operator_delete(old_ptr);
    }
  }
};

}  // namespace sfc::collections
