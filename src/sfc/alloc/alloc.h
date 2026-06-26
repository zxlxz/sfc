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

using Global = System;

}  // namespace sfc::alloc
