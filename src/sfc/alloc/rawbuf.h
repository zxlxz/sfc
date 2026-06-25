#pragma once

#include "sfc/alloc/alloc.h"

namespace sfc::alloc {

template <class T>
class RawBuf {
  static constexpr usize kMinCap = sizeof(T) <= sizeof(u32) ? 8UL : sizeof(T) <= 32 ? 4UL : 1UL;

  T* _ptr{nullptr};
  usize _cap{0};
  Allocator _a{alloc::global()};

 public:
  [[gnu::always_inline]] RawBuf() noexcept = default;

  [[gnu::always_inline]] ~RawBuf() noexcept {
    if (!_ptr) return;

    const auto layout = Layout{}.array<T>(_cap);
    _a.dealloc(_ptr, layout);
  }

  [[gnu::always_inline]] RawBuf(RawBuf&& other) noexcept
      : _ptr{mem::take(other._ptr)}, _cap{mem::take(other._cap)}, _a{other._a} {}

  [[gnu::always_inline]] RawBuf& operator=(RawBuf&& other) noexcept {
    if (this != &other) {
      mem::swap(_ptr, other._ptr);
      mem::swap(_cap, other._cap);
      mem::swap(_a, other._a);
    }
    return *this;
  }

  static auto with_capacity(usize capacity, Allocator alloc = alloc::global()) noexcept -> RawBuf {
    const auto layout = Layout{}.array<T>(capacity);

    auto res = RawBuf{};
    res._ptr = ptr::cast<T>(alloc.alloc(layout));
    res._cap = capacity;
    res._a = alloc;
    return res;
  }

  [[gnu::always_inline]] auto ptr() const noexcept -> T* {
    return _ptr;
  }

  [[gnu::always_inline]] auto cap() const noexcept -> usize {
    return _cap;
  }

  [[gnu::always_inline]] auto operator[](usize idx) const noexcept -> const T& {
    return _ptr[idx];
  }

  [[gnu::always_inline]] auto operator[](usize idx) noexcept -> T& {
    return _ptr[idx];
  }

  auto allocator() -> Allocator {
    return _a;
  }

  void reserve(usize len, usize additional) noexcept {
    const auto req_cap = num::saturating_add(len, additional);
    if (req_cap <= _cap) {
      return;
    }

    const auto fit_cap = cmp::max(req_cap, _cap * 2);
    const auto new_cap = cmp::max(fit_cap, kMinCap);
    const auto layout = Layout{}.array<T>(_cap);

    _ptr = ptr::cast<T>(_a.grow(_ptr, layout, new_cap * sizeof(T)));
    _cap = new_cap;
  }

  void reserve_exact(usize len, usize additional) noexcept {
    const auto new_cap = num::saturating_add(len, additional);
    if (new_cap <= _cap) {
      return;
    }

    const auto layout = Layout{}.array<T>(_cap);
    _ptr = ptr::cast<T>(_a.grow(_ptr, layout, new_cap * sizeof(T)));
    _cap = new_cap;
  }

  void shrink_to(usize new_cap) noexcept {
    if (new_cap >= _cap) {
      return;
    }

    const auto layout = Layout{}.array<T>(_cap);
    _ptr = ptr::cast<T>(_a.shrink(_ptr, layout, new_cap * sizeof(T)));
    _cap = new_cap;
  }
};

}  // namespace sfc::alloc

namespace sfc {
using alloc::RawBuf;
}  // namespace sfc
