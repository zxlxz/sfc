#include "sfc/test/test.h"

namespace sfc::alloc::test {

SFC_TEST(global_alloc_dealloc) {
  auto a = alloc::Global{};

  const auto layout = Layout{}.array<u64>(16);

  auto* ptr = a.alloc(layout);
  sfc::assert_ne(ptr, nullptr);

  a.dealloc(ptr, layout);
}

SFC_TEST(global_grow) {
  auto a = alloc::Global{};

  const auto layout = Layout{}.array<u64>(16);
  auto* ptr = a.alloc(layout);
  sfc::assert_ne(ptr, nullptr);

  auto* new_ptr = a.grow(ptr, layout, 32);
  sfc::assert_ne(new_ptr, nullptr);
  a.dealloc(new_ptr, Layout{}.array<u64>(32));
}

SFC_TEST(global_shrink) {
  auto a = alloc::Global{};

  const auto layout = Layout{}.array<u64>(32);
  auto* ptr = a.alloc(layout);
  sfc::assert_ne(ptr, nullptr);

  auto* new_ptr = a.shrink(ptr, layout, 16);
  sfc::assert_ne(new_ptr, nullptr);
  a.dealloc(new_ptr, Layout{}.array<u64>(16));
}

}  // namespace sfc::alloc::test
