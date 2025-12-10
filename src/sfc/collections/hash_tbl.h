#pragma once

#include "sfc/alloc/alloc.h"

namespace sfc::collections::hash_tbl {

static constexpr u8 CTRL_NUL = 0x80;
static constexpr u8 CTRL_DEL = 0xFF;

template <class T, class A = alloc::Global>
class HashTbl {
  u8* _ptr = nullptr;
  usize _cap = 0;
  usize _len = 0;
  usize _rem = 0;
  [[no_unique_address]] A _alloc{};

 public:
  HashTbl() noexcept = default;

  ~HashTbl() noexcept {
    this->dealloc();
  }

  HashTbl(HashTbl&& other) noexcept
      : _ptr{mem::take(other._ptr)}
      , _cap{mem::take(other._cap)}
      , _len{mem::take(other._len)}
      , _rem{mem::take(other._rem)} {}

  HashTbl& operator=(HashTbl&& other) noexcept {
    if (this != &other) {
      this->dealloc();
      _ptr = mem::take(other._ptr);
      _cap = mem::take(other._cap);
      _len = mem::take(other._len);
      _rem = mem::take(other._rem);
    }
    return *this;
  }

  auto len() const noexcept -> usize {
    return _len;
  }

  auto capacity() const noexcept -> usize {
    return _cap;
  }

  auto search(const auto& key) const -> T* {
    if (_len == 0) {
      return nullptr;
    }

    const auto ctrl_ptr = _ptr;
    const auto data_ptr = this->data();
    const auto [h1, h2] = this->hidx(key);
    for (auto idx = h1;; idx = (idx + 1) & (_cap - 1)) {
      const auto ctrl = ctrl_ptr[idx];
      const auto& item = data_ptr[idx];
      if (ctrl == CTRL_NUL) {
        return nullptr;
      } else if (ctrl == h2 && item.key == key) {
        return &item;
      }
    }
    return nullptr;
  }

  auto search_or_insert(T&& entry) -> T* {
    this->reserve(1);
    if (_len == 0) {
      return nullptr;
    }

    const auto ctrl_ptr = _ptr;
    const auto data_ptr = this->data();
    const auto [h1, h2] = this->hidx(key);
    auto del_idx = _cap;
    for (auto idx = h1;; idx = (idx + 1) & (_cap - 1)) {
      const auto ctrl = _ptr[idx];
      const auto& item = data_ptr[idx];
      if (ctrl == CTRL_NUL) {
        const auto ins_idx = del_idx == _cap ? idx : del_idx;
        this->insert_at(ins_idx, static_cast<T&&>(entry));
        return nullptr;
      } else if (ctrl == h2 && item.key == key) {
        return &item;  // has_value
      } else if (ctrl == CTRL_DEL && del_idx == _cap) {
        del_idx = idx;
      }
    }
    return nullptr;
  }

  void clear() noexcept {
    if (_len == 0) {
      return;
    }
    this->for_each([&](T& val) { val.~T(); });
    ptr::write_bytes(_ptr, CTRL_NUL, _mask + 1);
    _rem += _len;
    _len = 0;
  }

  void reserve(usize additional) {
    if (additional < _rem) {
      return;
    }

    const auto old_ctrl = _ptr;
    const auto old_data = this->data();
    const auto old_cap = _cap;
    const auto old_len = _len;

    this->alloc(old_cap + additional);
    if (old_len == 0) {
      return;
    }

    for (auto i = 0UL; i < old_cap; ++i) {
      if (old_ctrl[i] < CTRL_NUL) {
        auto& tmp = old_data[i];
        this->insert_new(new_data, tmp.key, static_cast<T&&>(tmp));
        tmp.~T();
      }
    }
  }

  void for_each(this auto& self, auto&& f) {
    if (self._len == 0) {
      return;
    }

    const auto ctrl = self._ctrl;
    const auto data = self.data();
    for (auto i = 0UL; i < self._mask + 1; ++i) {
      if (ctrl[i] < CTRL_NUL) {
        f(data[i]);
      }
    }
  }

 private:
  static constexpr auto ALIGN = 128;
  static constexpr auto MAX_LOAD_FACTOR = 0.75f;

  struct HIdx {
    u64 h1;
    u8 h2;
  };

  auto hidx(const auto& key) const noexcept -> HIdx {
    const auto hx = [&]() {
      if constexpr (requires { key.hash(); }) {
        return key.hash();
      } else {
        return static_cast<u64>(key);
      }
    }();
    return {hx & _mask, static_cast<u8>((hx >> 57) & 0x7F)};
  }

  auto data() const noexcept -> T* {
    const auto ctrl_size = (_mask + ALIGN) & ~(ALIGN - 1);
    return reinterpret_cast<T*>(_ptr + ctrl_size);
  }

  auto layout() const noexcept -> alloc::Layout {
    const auto ctrl_size = (_mask + ALIGN) & ~(ALIGN - 1);
    const auto data_size = (_mask + 1) * sizeof(T);
    return {ctrl_size + data_size, alignof(T)};
  }

  void alloc(usize min_cap) {
    auto cap = 16U;
    while (cap < min_cap) {
      cap *= 2;
    }
    _mask = cap - 1;
    _ptr = static_cast<u8*>(A::alloc(this->layout()));
    _len = 0;
    _rem = static_cast<usize>(cap * MAX_LOAD_FACTOR);
    ptr::write_bytes(_ptr, CTRL_NUL, cap);
  }

  void dealloc() {
    this->clear();
    A::dealloc(_ptr, this->layout());
  }

  void insert_at(usize idx, T&& val) noexcept {
    const auto dst = this->data() + idx;
    ptr::write(dst, static_cast<T&&>(val));
    _ptr[idx] = h2;
    _len += 1;
    _rem -= 1;
  }
};

}  // namespace sfc::collections::hash_tbl
