#pragma once

#include "sfc/core.h"

namespace sfc::alloc {

using mem::Layout;

struct Global {
  static auto alloc(Layout layout) noexcept -> void*;
  static void dealloc(void* ptr, Layout layout) noexcept;
  static auto realloc(void* ptr, Layout layout, usize new_size) noexcept -> void*;

  static auto grow(void* ptr, Layout old_layout, Layout new_layout) noexcept -> void*;
  static auto shrink(void* ptr, Layout old_layout, Layout new_layout) noexcept -> void*;
};

template <class T, class A = alloc::Global>
class RawBuf {
  static constexpr usize kMinCap = sizeof(T) == 1 ? 8UL : sizeof(T) <= 256 ? 4UL : 1UL;

  T* _ptr{nullptr};
  usize _cap{0};
  [[no_unique_address]] A _a{};

 public:
  [[gnu::always_inline]] RawBuf() noexcept = default;

  [[gnu::always_inline]] ~RawBuf() noexcept {
    if (!_ptr) return;

    const auto layout = alloc::Layout::template array<T>(_cap);
    _a.dealloc(_ptr, layout);
  }

  [[gnu::always_inline]] RawBuf(RawBuf&& other) noexcept
      : _ptr{mem::take(other._ptr)}, _cap{mem::take(other._cap)}, _a{mem::move(other._a)} {}

  [[gnu::always_inline]] RawBuf& operator=(RawBuf&& other) noexcept {
    if (this != &other) {
      mem::swap(_ptr, other._ptr);
      mem::swap(_cap, other._cap);
      mem::swap(_a, other._a);
    }
    return *this;
  }

  static auto with_capacity(usize capacity, A alloc = {}) noexcept -> RawBuf {
    const auto layout = alloc::Layout::template array<T>(capacity);

    auto res = RawBuf{};
    res._ptr = static_cast<T*>(alloc.alloc(layout));
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

  auto allocator() -> A& {
    return _a;
  }

  void reserve(usize len, usize additional) noexcept {
    if (additional < _cap - len) {
      return;
    }

    const auto req_cap = len + additional;
    const auto fit_cap = cmp::max(req_cap, _cap * 2);
    const auto new_cap = cmp::max(fit_cap, kMinCap);

    const auto old_layout = alloc::Layout::template array<T>(_cap);
    const auto new_layout = alloc::Layout::template array<T>(new_cap);
    _ptr = static_cast<T*>(_a.grow(_ptr, old_layout, new_layout));
    _cap = new_cap;
  }

  void reserve_exact(usize len, usize additional) noexcept {
    if (additional < _cap - len) {
      return;
    }

    const auto new_cap = len + additional;
    const auto old_layout = alloc::Layout::template array<T>(_cap);
    const auto new_layout = alloc::Layout::template array<T>(new_cap);
    _ptr = static_cast<T*>(_a.grow(_ptr, old_layout, new_layout));
    _cap = new_cap;
  }

  void shrink_to(usize new_cap) noexcept {
    if (new_cap >= _cap) {
      return;
    }

    const auto old_layout = alloc::Layout::template array<T>(_cap);
    const auto new_layout = alloc::Layout::template array<T>(new_cap);
    _ptr = static_cast<T*>(_a.shrink(_ptr, old_layout, new_layout));
    _cap = new_cap;
  }
};

}  // namespace sfc::alloc

namespace sfc {
using alloc::Layout;
using alloc::RawBuf;
}  // namespace sfc
