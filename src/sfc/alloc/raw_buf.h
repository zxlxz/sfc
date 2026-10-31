#pragma once

#include "sfc/alloc/alloc.h"

namespace sfc::raw_buf {

template <class T, class A = alloc::Global>
class RawBuf {
  static constexpr usize kMinCap = sizeof(T) <= 4 ? 8UL : sizeof(T) <= 32 ? 4UL : 1UL;
  static constexpr usize kMaxCap = usize{1ULL << 40};  // 1TB

  T* _ptr{nullptr};
  usize _cap{0};
  [[no_unique_address]] A _a{};

 public:
  explicit RawBuf(A alloc = {}) noexcept : _a{alloc} {}

  ~RawBuf() noexcept {
    if (!_ptr) return;

    const auto layout = Layout::array<T>(_cap);
    _a.deallocate(_ptr, layout);
  }

  RawBuf(RawBuf&& other) noexcept : _ptr{mem::take(other._ptr)}, _cap{mem::take(other._cap)}, _a{mem::move(other._a)} {}

  RawBuf& operator=(RawBuf&& other) noexcept {
    if (this != &other) {
      mem::swap(_ptr, other._ptr);
      mem::swap(_cap, other._cap);
      mem::swap(_a, other._a);
    }
    return *this;
  }

  static auto with_capacity(usize capacity, A alloc = {}) -> RawBuf {
    auto res = RawBuf{alloc};
    res.reserve_exact(0, capacity);
    return res;
  }

  static auto with_capacity_zeroed(usize capacity, A alloc = {}) -> RawBuf {
    const auto ptr = alloc.allocate_zeroed(Layout::array<T>(capacity));

    auto res = RawBuf{alloc};
    res.set_ptr_and_cap(ptr, capacity);
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

 public:
  void reserve(usize len, usize additional) noexcept {
    sfc::assert_(additional <= kMaxCap, "RawBuf::reserve: additional(={}) too large", additional);
    if (additional <= _cap - len) {
      return;
    }

    const auto min_cap = cmp::max(kMinCap, 2 * _cap);
    const auto new_cap = cmp::max(len + additional, min_cap);
    this->grow_unchecked(len, new_cap - len);
  }

  void reserve_exact(usize len, usize additional) noexcept {
    sfc::assert_(additional <= kMaxCap, "RawBuf::reserve_exact: additional(={}) too large", additional);
    if (additional <= _cap - len) {
      return;
    }

    this->grow_unchecked(len, additional);
  }

  void shrink_to_fit(usize new_cap) noexcept {
    if (new_cap >= _cap) {
      return;
    }

    this->shrink_unchecked(new_cap);
  }

 private:
  void set_ptr_and_cap(void* ptr, usize cap) noexcept {
    _ptr = ptr::cast<T>(ptr);
    _cap = cap;
  }

  void shrink_unchecked(usize new_cap) noexcept {
    const auto old_layout = Layout::array<T>(_cap);
    const auto new_layout = Layout::array<T>(new_cap);
    const auto new_ptr = _a.shrink(_ptr, old_layout, new_layout);
    this->set_ptr_and_cap(new_ptr, new_cap);
  }

  void grow_unchecked(usize len, usize additional) noexcept {
    const auto old_layout = Layout::array<T>(_cap);
    const auto new_layout = Layout::array<T>(len + additional);
    const auto new_ptr = _a.grow(_ptr, old_layout, new_layout);
    this->set_ptr_and_cap(new_ptr, len + additional);
  }
};

}  // namespace sfc::raw_buf
