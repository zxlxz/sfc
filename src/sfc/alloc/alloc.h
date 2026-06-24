#pragma once

#include "sfc/core.h"

namespace sfc::alloc {

using mem::Layout;

class Allocator {
 protected:
  Allocator() noexcept;
  ~Allocator() noexcept;

  Allocator(const Allocator&) noexcept = delete;
  Allocator& operator=(const Allocator&) noexcept = delete;

 public:
  virtual void* allocate(Layout layout) = 0;
  virtual void deallocate(void* ptr, Layout layout) = 0;

  virtual void* grow(void* ptr, Layout old_layout, usize new_size);
  virtual void* shrink(void* ptr, Layout old_layout, usize new_size);
};

class Global : public Allocator {
  Global();
  ~Global();

 public:
  static auto instance() noexcept -> Global&;

  void* allocate(Layout layout) override;
  void deallocate(void* ptr, Layout layout) override;

  void* grow(void* ptr, Layout layout, usize new_size) override;
  void* shrink(void* ptr, Layout layout, usize new_size) override;
};

template <class T>
class RawBuf {
  T* _ptr{nullptr};
  usize _cap{0};
  Allocator* _a{&alloc::Global::instance()};

 public:
  [[gnu::always_inline]] RawBuf() noexcept = default;

  [[gnu::always_inline]] ~RawBuf() noexcept {
    if (!_ptr) return;

    const auto layout = Layout{}.array<T>(_cap);
    _a->deallocate(_ptr, layout);
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

  static auto with_capacity(usize capacity, Allocator& alloc = Global::instance()) noexcept -> RawBuf {
    const auto layout = Layout{}.array<T>(capacity);

    auto res = RawBuf{};
    res._ptr = ptr::cast<T>(alloc.allocate(layout));
    res._cap = capacity;
    res._a = &alloc;
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

  auto allocator() -> Allocator& {
    return *_a;
  }

  void reserve(usize len, usize additional) noexcept {
    static constexpr usize kMinCap = sizeof(T) == 1 ? 8UL : sizeof(T) <= 256 ? 4UL : 1UL;
    if (additional < _cap - len) {
      return;
    }

    const auto req_cap = len + additional;
    const auto fit_cap = cmp::max(req_cap, _cap * 2);
    const auto new_cap = cmp::max(fit_cap, kMinCap);
    const auto layout = Layout{}.array<T>(_cap);

    _ptr = ptr::cast<T>(_a->grow(_ptr, layout, new_cap * sizeof(T)));
    _cap = new_cap;
  }

  void reserve_exact(usize len, usize additional) noexcept {
    if (additional < _cap - len) {
      return;
    }

    const auto layout = Layout{}.array<T>(_cap);
    const auto new_cap = len + additional;
    _ptr = ptr::cast<T>(_a->grow(_ptr, layout, new_cap * sizeof(T)));
    _cap = new_cap;
  }

  void shrink_to(usize new_cap) noexcept {
    if (new_cap >= _cap) {
      return;
    }

    const auto layout = Layout{}.array<T>(_cap);
    _ptr = ptr::cast<T>(_a->shrink(_ptr, layout, new_cap * sizeof(T)));
    _cap = new_cap;
  }
};

}  // namespace sfc::alloc

namespace sfc {
using alloc::Layout;
using alloc::RawBuf;
using alloc::Allocator;
}  // namespace sfc
