#if defined(__unix__) || defined(__APPLE__)
#include "sfc/sys/unix/alloc.inl"
#elif defined(_WIN32)
#include "sfc/sys/windows/alloc.inl"
#endif

#include "sfc/alloc/alloc.h"

namespace sfc::alloc {

Allocator::Allocator() noexcept {}

Allocator::~Allocator() noexcept {}

void* Allocator::grow(void* ptr, Layout layout, usize new_size) {
  if (new_size <= layout.size) {
    return ptr;
  }

  const auto new_ptr = this->allocate({new_size, layout.align});
  if (ptr && new_ptr) {
    __builtin_memcpy(new_ptr, ptr, layout.size);
  }
  this->deallocate(ptr, layout);
  return new_ptr;
}

void* Allocator::shrink(void* ptr, Layout layout, usize new_size) {
  if (new_size >= layout.size) {
    return ptr;
  }

  const auto new_ptr = this->allocate({new_size, layout.align});
  if (ptr && new_ptr) {
    __builtin_memcpy(new_ptr, ptr, new_size);
  }
  this->deallocate(ptr, layout);
  return new_ptr;
}

Global::Global() {}

Global::~Global() {}

auto Global::instance() noexcept -> Global& {
  static auto g = Global{};
  return g;
}

void* Global::allocate(Layout layout) {
  if (layout.size == 0) {
    return nullptr;
  }

  return sys::alloc(layout);
}

void Global::deallocate(void* ptr, Layout layout) {
  if (ptr == nullptr) {
    return;
  }

  sys::dealloc(ptr, layout);
}

void* Global::grow(void* ptr, Layout layout, usize new_size) {
  if (layout.size >= new_size) {
    return ptr;
  }

  if (layout.size == 0) {
    return sys::alloc(Layout{new_size, layout.align});
  }

  return sys::realloc(ptr, layout, new_size);
}

void* Global::shrink(void* ptr, Layout layout, usize new_size) {
  if (layout.size <= new_size) {
    return ptr;
  }

  if (new_size == 0) {
    sys::dealloc(ptr, layout);
    return nullptr;
  }

  return sys::realloc(ptr, layout, new_size);
}

}  // namespace sfc::alloc
