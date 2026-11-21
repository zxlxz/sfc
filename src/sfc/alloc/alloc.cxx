#include "sfc/test.h"
#include "sfc/alloc/alloc.h"

namespace sfc::alloc::test {

SFC_TEST(fixed_buf_allocate) {
  u8 storage[32]{};
  auto allocator = FixedBuf{Slice<u8>{storage}, 0};

  sfc::assert_eq(allocator.allocate(Layout{.size = 0, .align = 1}), nullptr);

  const auto layout = Layout{.size = 4, .align = alignof(u32)};
  auto* ptr = allocator.allocate(layout);
  sfc::assert_ne(ptr, nullptr);
  sfc::assert_eq((usize)ptr % layout.align, 0U);
  sfc::assert_eq(allocator.is_last_allocation(ptr, layout), true);

  auto* bytes = ptr::cast<u8>(ptr);
  bytes[0] = 1;
  bytes[1] = 2;
  sfc::assert_eq(bytes[0], 1);
  sfc::assert_eq(bytes[1], 2);
  sfc::assert_eq(allocator.allocate(Layout{.size = 64, .align = 1}), nullptr);
}

SFC_TEST(fixed_buf_allocate_zeroed) {
  u8 storage[16];
  for (auto& byte : storage) {
    byte = 0xff;
  }
  auto allocator = FixedBuf{Slice<u8>{storage}, 0};

  auto* ptr = allocator.allocate_zeroed(Layout{.size = 8, .align = 1});
  sfc::assert_ne(ptr, nullptr);
  auto* bytes = ptr::cast<u8>(ptr);
  for (auto i = 0U; i < 8; ++i) {
    sfc::assert_eq(bytes[i], 0);
  }
}

SFC_TEST(fixed_buf_deallocate) {
  u8 storage[32]{};
  auto allocator = FixedBuf{Slice<u8>{storage}, 0};
  const auto layout = Layout{.size = 4, .align = 1};

  auto* first = allocator.allocate(layout);
  auto* second = allocator.allocate(layout);
  sfc::assert_ne(first, nullptr);
  sfc::assert_ne(second, nullptr);
  sfc::assert_eq(allocator.is_last_allocation(first, layout), false);

  allocator.deallocate(first, layout);
  sfc::assert_eq(allocator.is_last_allocation(second, layout), true);
  allocator.deallocate(second, layout);
  sfc::assert_eq(allocator.is_last_allocation(second, layout), false);
}

SFC_TEST(fixed_buf_grow_shrink) {
  u8 storage[32]{};
  auto allocator = FixedBuf{Slice<u8>{storage}, 0};
  const auto layout = Layout{.size = 4, .align = 1};

  auto* ptr = allocator.allocate(layout);
  sfc::assert_ne(ptr, nullptr);
  auto* bytes = ptr::cast<u8>(ptr);
  bytes[0] = 7;
  bytes[1] = 9;

  auto* grown = allocator.grow(ptr, layout, 12);
  sfc::assert_eq(grown, ptr);
  sfc::assert_eq(bytes[0], 7);
  sfc::assert_eq(bytes[1], 9);

  const auto grown_layout = Layout{.size = 12, .align = 1};
  auto* shrunk = allocator.shrink(grown, grown_layout, 2);
  sfc::assert_eq(shrunk, ptr);
  sfc::assert_eq(allocator.is_last_allocation(shrunk, Layout{.size = 2, .align = 1}), true);
}

SFC_TEST(fixed_buf_grow_non_last) {
  u8 storage[32]{};
  auto allocator = FixedBuf{Slice<u8>{storage}, 0};
  const auto layout = Layout{.size = 4, .align = 1};

  auto* first = allocator.allocate(layout);
  auto* second = allocator.allocate(layout);
  sfc::assert_ne(first, nullptr);
  sfc::assert_ne(second, nullptr);
  auto* first_bytes = ptr::cast<u8>(first);
  first_bytes[0] = 11;
  first_bytes[1] = 13;

  auto* grown = allocator.grow(first, layout, 8);
  sfc::assert_ne(grown, nullptr);
  sfc::assert_ne(grown, first);
  auto* grown_bytes = ptr::cast<u8>(grown);
  sfc::assert_eq(grown_bytes[0], 11);
  sfc::assert_eq(grown_bytes[1], 13);
}

SFC_TEST(global_alloc_dealloc) {
  auto a = alloc::Global{};

  const auto layout = Layout::array<u64>(16);

  auto* ptr = a.allocate(layout);
  sfc::assert_ne(ptr, nullptr);

  a.deallocate(ptr, layout);
}

SFC_TEST(global_grow) {
  auto a = alloc::Global{};

  const auto layout = Layout::array<u64>(16);
  auto* ptr = a.allocate(layout);
  sfc::assert_ne(ptr, nullptr);

  auto* new_ptr = a.grow(ptr, layout, 32);
  sfc::assert_ne(new_ptr, nullptr);
  a.deallocate(new_ptr, Layout::array<u64>(32));
}

SFC_TEST(global_shrink) {
  auto a = alloc::Global{};

  const auto layout = Layout::array<u64>(32);
  auto* ptr = a.allocate(layout);
  sfc::assert_ne(ptr, nullptr);

  auto* new_ptr = a.shrink(ptr, layout, 16);
  sfc::assert_ne(new_ptr, nullptr);
  a.deallocate(new_ptr, Layout{}.array<u64>(16));
}

}  // namespace sfc::alloc::test
