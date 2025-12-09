#pragma once

#include "sfc/alloc/alloc.h"

namespace sfc::collections::hash_tbl {

auto hash(const auto& x) noexcept -> usize {
  if constexpr (requires { x.hash(); }) {
    return x.hash();
  } else {
    return x;
  }
}

template <class T, class A = alloc::Global>
class HashTbl {
  static constexpr u8 CTRL_NUL = 0x80;
  static constexpr u8 CTRL_DEL = 0xFF;

  static constexpr auto CTRL_ALIGN = 128U;
  static constexpr auto MAX_LOAD_FACTOR = 0.75;

  u8* _ctrl = nullptr;
  usize _mask = 0;
  usize _len = 0;
  usize _rem = 0;
  [[no_unique_address]] A _alloc{};

 public:
  HashTbl() noexcept = default;

  ~HashTbl() {
    this->dealloc();
  }

  HashTbl(HashTbl&& other) noexcept
      : _ctrl{mem::take(other._ctrl)}
      , _mask{mem::take(other._mask)}
      , _len{mem::take(other._len)}
      , _rem{mem::take(other._rem)} {}

  HashTbl& operator=(HashTbl&& other) noexcept {
    if (this != &other) {
      this->dealloc();
      _ctrl = mem::take(other._ctrl);
      _mask = mem::take(other._mask);
      _rem = mem::take(other._rem);
      _len = mem::take(other._len);
    }
    return *this;
  }

  auto len() const noexcept -> usize {
    return _len;
  }

  auto capacity() const noexcept -> usize {
    return _mask == 0 ? 0 : _mask + 1;
  }

  auto search(const auto& key) const -> T* {
    if (_len == 0) {
      return nullptr;
    }

    const auto [h1, h2] = this->hidx(key);
    const auto data = this->data();
    for (auto idx = h1;; idx = (idx + 1) & _mask) {
      const auto ctrl = _ctrl[idx];
      if (ctrl == CTRL_NUL) {
        return nullptr;
      } else if (ctrl == h2 && data[idx].key == key) {
        return &data[idx];
      }
    }
    return nullptr;
  }

  auto search_or_insert(T&& entry) -> T* {
    if (_rem == 0) {
      this->reserve(1);
    }

    const auto [h1, h2] = this->hidx(entry.key);
    const auto data = this->data();

    auto del_idx = _mask + 1;
    for (auto idx = h1;; idx = (idx + 1) & _mask) {
      const auto ctrl = _ctrl[idx];
      if (ctrl == CTRL_NUL) {
        const auto ins_idx = del_idx == _mask + 1 ? idx : del_idx;
        this->insert_at(data, ins_idx, h2, static_cast<T&&>(entry));
        return nullptr;
      } else if (ctrl == CTRL_DEL) {
        if (del_idx == _mask + 1) {
          del_idx = idx;
        }
      } else if (ctrl == h2 && data[idx].key == entry.key) {
        return &data[idx];
      }
    }
  }

  auto erase(const T* dest) noexcept -> bool {
    const auto data = this->data();
    const auto idx = static_cast<usize>(dest - data);
    if (idx > _mask) {
      return false;
    }
    this->erase_at(data, idx);
    return true;
  }

  void clear() noexcept {
    if (_ctrl == nullptr || _len == 0) {
      return;
    }

    _rem = _rem + _len;
    _len = 0;

    const auto data = this->data();
    for (auto i = 0UL; i < _mask + 1; ++i) {
      if (_ctrl[i] < CTRL_NUL) {
        data[i].~T();
      }
    }
    ptr::write_bytes(_ctrl, CTRL_NUL, _mask + 1);
  }

  void reserve(usize additional) {
    if (additional < _rem) {
      return;
    }

    auto old = mem::move(*this);
    const auto old_cap = old.capacity();
    const auto old_data = old.data();

    this->alloc(old_cap + additional);
    if (old._ctrl == nullptr) {
      return;
    }

    const auto new_data = this->data();
    for (auto i = 0UL; i < old_cap; ++i) {
      if (old._ctrl[i] >= CTRL_NUL) {
        continue;
      }
      auto& tmp = old_data[i];
      this->insert_new(new_data, tmp.key, static_cast<T&&>(tmp));
      tmp.~T();
    }
    old._len = 0;
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
  struct HIdx {
    u64 h1;
    u8 h2;
  };
  auto hidx(const auto& key) const -> HIdx {
    const auto hx = hash(key);
    return {hx & _mask, static_cast<u8>((hx >> 57) & 0x7F)};
  }

  auto data() const -> T* {
    const auto offs = (_mask + CTRL_ALIGN) & ~(CTRL_ALIGN - 1);
    return reinterpret_cast<T*>(_ctrl + offs);
  }

  auto layout() const -> alloc::Layout {
    const auto cap = _mask + 1;
    const auto offs = (cap - 1 + CTRL_ALIGN) & ~(CTRL_ALIGN - 1);
    const auto size = offs + (cap * sizeof(T));
    return {size, CTRL_ALIGN};
  }

  void alloc(usize min_cap) {
    auto cap = 16U;
    while (cap < min_cap) {
      cap *= 2;
    }
    _mask = cap - 1;
    _ctrl = static_cast<u8*>(A::alloc(this->layout()));
    _len = 0;
    _rem = static_cast<usize>(static_cast<f64>(cap) * MAX_LOAD_FACTOR);
    ptr::write_bytes(_ctrl, CTRL_NUL, cap);
  }

  void dealloc() {
    this->clear();
    A::dealloc(_ctrl, this->layout());
  }

  auto insert_new(T* data, const auto& key, T&& val) -> bool {
    const auto [h1, h2] = this->hidx(key);

    for (auto idx = h1;; idx = (idx + 1) & _mask) {
      if (_ctrl[idx] == CTRL_NUL) {
        this->insert_at(data, idx, h2, static_cast<T&&>(val));
        return true;
      }
    }
    return false;
  }

  void insert_at(T* data, usize idx, u8 h2, T&& val) {
    _ctrl[idx] = h2;
    new (&data[idx]) T{static_cast<T&&>(val)};
    _len += 1;
    _rem -= 1;
  }

  void erase_at(T* data, usize idx) {
    _ctrl[idx] = CTRL_NUL;
    data[idx].~T();
    _len -= 1;
  }
};

}  // namespace sfc::collections::hash_tbl
