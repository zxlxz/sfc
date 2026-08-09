#pragma once

#include "sfc/alloc/alloc.h"

namespace sfc::buffer {

template <class T, class A = alloc::Global>
class Buffer {
  T* _ptr{nullptr};
  usize _cap{0};
  [[no_unique_address]] A _a{};

 public:
  Buffer(A alloc = {}) noexcept : _a{mem::move(alloc)} {}

  ~Buffer() noexcept {
    if (!_ptr) return;
    _a.deallocate(_ptr, this->layout());
  }

  Buffer(Buffer&& other) noexcept : _ptr{mem::take(other._ptr)}, _cap{mem::take(other._cap)}, _a{mem::move(other._a)} {}

  Buffer& operator=(Buffer&& other) noexcept {
    if (this != &other) {
      mem::swap(_ptr, other._ptr);
      mem::swap(_cap, other._cap);
      mem::swap(_a, other._a);
    }
    return *this;
  }

  static auto with_capacity(usize capacity, A alloc = {}) -> Buffer {
    auto res = Buffer{};
    res._a = alloc;
    res._cap = capacity;
    res._ptr = ptr::cast<T>(res._a.allocate(res.layout()));
    return res;
  }

  static auto with_capacity_zeroed(usize capacity, A alloc={}) -> Buffer {
    auto res = Buffer{};
    res._a = alloc;
    res._cap = capacity;
    res._ptr = ptr::cast<T>(res._a.alloc_zeroed(res.layout()));
    return res;
  }

 public:
  auto ptr() const noexcept -> T* {
    return _ptr;
  }

  auto cap() const noexcept -> usize {
    return _cap;
  }

  auto operator[](usize idx) const noexcept -> const T& {
    return _ptr[idx];
  }

  auto operator[](usize idx) noexcept -> T& {
    return _ptr[idx];
  }

  auto allocator() -> A& {
    return _a;
  }

  auto layout() const -> mem::Layout {
    return mem::Layout::array<T>(_cap);
  }

 public:
  void reserve(usize len, usize additional) noexcept {
    constexpr usize kMinCap = sizeof(T) <= 4 ? 8UL : sizeof(T) <= 32 ? 4UL : 1UL;
    constexpr usize kMaxCap = num::Int<u32>::MAX;
    sfc::assert_(additional <= kMaxCap, "RawBuf::reserve: additional(={}) too large", additional);

    const auto req_cap = num::saturating_add(len, additional);
    if (req_cap <= _cap) {
      return;
    }

    const auto fit_cap = cmp::max(req_cap, _cap * 2);
    const auto new_cap = cmp::max(fit_cap, kMinCap);
    const auto layout = this->layout();

    _ptr = ptr::cast<T>(_a.grow(_ptr, layout, new_cap * sizeof(T)));
    _cap = new_cap;
  }

  void reserve_exact(usize len, usize additional) noexcept {
    const auto new_cap = num::saturating_add(len, additional);
    if (new_cap <= _cap) {
      return;
    }

    const auto layout = this->layout();
    _ptr = ptr::cast<T>(_a.grow(_ptr, layout, new_cap * sizeof(T)));
    _cap = new_cap;
  }

  void shrink_to(usize new_cap) noexcept {
    if (new_cap >= _cap) {
      return;
    }

    const auto layout = this->layout();
    _ptr = ptr::cast<T>(_a.shrink(_ptr, layout, new_cap * sizeof(T)));
    _cap = new_cap;
  }
};

}  // namespace sfc::buffer

namespace sfc {
using buffer::Buffer;
}  // namespace sfc
