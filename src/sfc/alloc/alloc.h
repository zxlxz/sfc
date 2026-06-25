#pragma once

#include "sfc/core.h"

namespace sfc::alloc {

using mem::Layout;

class IAlloc {
 public:
  virtual void* alloc(Layout layout) = 0;
  virtual void dealloc(void* ptr, Layout layout) = 0;

  virtual void* grow(void* ptr, Layout old_layout, usize new_size);
  virtual void* shrink(void* ptr, Layout old_layout, usize new_size);
};

class System : public IAlloc {
 public:
  void* alloc(Layout layout) override;
  void dealloc(void* ptr, Layout layout) override;

  void* grow(void* ptr, Layout layout, usize new_size) override;
  void* shrink(void* ptr, Layout layout, usize new_size) override;
};

class Allocator {
  IAlloc* _alloc{nullptr};

 public:
  Allocator(IAlloc& alloc) noexcept : _alloc{&alloc} {}
  ~Allocator() noexcept = default;

 public:
  void* alloc(Layout layout) {
    return _alloc->alloc(layout);
  }

  void dealloc(void* ptr, Layout layout) {
    return _alloc->dealloc(ptr, layout);
  }

  void* grow(void* ptr, Layout layout, usize new_size) {
    return _alloc->grow(ptr, layout, new_size);
  }

  void* shrink(void* ptr, Layout layout, usize new_size) {
    return _alloc->shrink(ptr, layout, new_size);
  }
};

auto global() -> IAlloc&;

}  // namespace sfc::alloc

namespace sfc {
using alloc::Allocator;
}  // namespace sfc
