#pragma once

#include "sfc/alloc/alloc.h"

namespace sfc::raw_buf {

template <class T, class A = alloc::Global>
class RawBuf {
  T* _ptr;
  usize _cap;
  [[no_unique_address]] A _a;

  explicit RawBuf(T* ptr, usize cap, A a) noexcept : _ptr{ptr}, _cap{cap}, _a{a} {}

 public:
  ~RawBuf() noexcept {
    if (_ptr == nullptr) {
      return;
    }
    _a.deallocate(_ptr, Layout::array<T>(_cap));
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

  static auto new_(A a = {}) -> RawBuf {
    return RawBuf{nullptr, 0, a};
  }

  static auto with_capacity(usize capacity, A a = {}) -> RawBuf {
    if (capacity == 0) {
      return RawBuf{nullptr, 0, a};
    }
    const auto p = (T*)a.allocate(Layout::array<T>(capacity));
    test::assert_(p != nullptr, "RawBuf::with_capacity(capacity={}): allocate failed", capacity);
    return RawBuf{p, capacity, a};
  }

  static auto with_capacity_zeroed(usize capacity, A a = {}) -> RawBuf {
    if (capacity == 0) {
      return RawBuf{nullptr, 0, a};
    }
    const auto p = (T*)a.allocate_zeroed(Layout::array<T>(capacity));
    test::assert_(p != nullptr, "RawBuf::with_capacity_zeroed(capacity={}): allocate failed", capacity);
    return RawBuf{p, capacity, a};
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

  auto allocator() const -> const A& {
    return _a;
  }

 public:
  void reserve(usize len, usize additional) noexcept {
    static constexpr usize kMinCap = sizeof(T) <= 4 ? 8UL : sizeof(T) <= 32 ? 4UL : 1UL;
    if (additional <= _cap - len) {
      return;
    }

    const auto min_cap = cmp::max(len + additional, _cap + (_cap + 1) / 2);
    const auto new_cap = cmp::max(kMinCap, min_cap);
    this->grow_unchecked(len, new_cap - len);
  }

  void reserve_exact(usize len, usize additional) noexcept {
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
    if (new_cap != 0) {
      test::assert_(new_ptr != nullptr, "RawBuf::shrink_unchecked(new_cap={}): allocate failed", new_cap);
    }

    this->set_ptr_and_cap(new_ptr, new_cap);
  }

  void grow_unchecked(usize len, usize additional) noexcept {
    const auto old_layout = Layout::array<T>(_cap);
    const auto new_layout = Layout::array<T>(len + additional);
    const auto new_ptr = _a.grow(_ptr, old_layout, new_layout);
    test::assert_(
        new_ptr != nullptr, "RawBuf::grow_unchecked(len={}, additional={}): allocate failed", len, additional);

    this->set_ptr_and_cap(new_ptr, len + additional);
  }
};

}  // namespace sfc::raw_buf

namespace sfc {
using raw_buf::RawBuf;
}
