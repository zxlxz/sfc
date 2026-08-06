#include "sfc/alloc/alloc.h"
#include "sfc/alloc/mpool.h"
#include "sfc/test/test.h"

namespace sfc::mpool::test {

// A fresh pool reports zero usage; allocate/dealloc round-trip is non-null.
SFC_TEST(mpool_alloc_basic) {
  auto pool = MPool<alloc::Global>{};

  sfc::assert_eq(pool.total_bytes(), 0U);
  sfc::assert_eq(pool.free_bytes(), 0U);

  auto* ptr = pool.allocate(64);
  sfc::assert_ne(ptr, nullptr);

  // Issuing a block increases outstanding bytes, nothing free yet.
  sfc::assert_ge(pool.total_bytes(), 64U);
  sfc::assert_eq(pool.free_bytes(), 0U);

  pool.dealloc(ptr, Layout{64, 1});
}

// After dealloc, the block lands in the free list: a subsequent allocate of
// the same size is served from the cache (free_bytes drops back to 0).
SFC_TEST(mpool_cache_reuse) {
  auto pool = MPool<alloc::Global>{};

  auto* p1 = pool.allocate(128);
  sfc::assert_ne(p1, nullptr);
  pool.dealloc(p1, Layout{128, 1});

  // The just-released block is now cached.
  sfc::assert_eq(pool.free_bytes(), 128U);

  auto* p2 = pool.allocate(128);
  sfc::assert_ne(p2, nullptr);
  sfc::assert_eq(pool.free_bytes(), 0U);

  pool.dealloc(p2, Layout{128, 1});
}

// Distinct sizes are tracked in separate buckets; freeing two different sizes
// yields the sum of both in free_bytes.
SFC_TEST(mpool_distinct_sizes) {
  auto pool = MPool<alloc::Global>{};

  auto* a = pool.allocate(64);
  auto* b = pool.allocate(256);
  sfc::assert_ne(a, nullptr);
  sfc::assert_ne(b, nullptr);

  pool.dealloc(a, Layout{64, 1});
  pool.dealloc(b, Layout{256, 1});

  sfc::assert_eq(pool.free_bytes(), 64U + 256U);

  // Re-allocating either size is served from cache.
  auto* a2 = pool.allocate(64);
  sfc::assert_ne(a2, nullptr);
  sfc::assert_eq(pool.free_bytes(), 256U);

  pool.dealloc(a2, Layout{64, 1});
}

// Repeated allocate/dealloc churn keeps the pool stable and non-null.
SFC_TEST(mpool_churn) {
  auto pool = MPool<alloc::Global>{};

  for (auto i = 0U; i < 1000; ++i) {
    auto* p = pool.allocate(64);
    sfc::assert_ne(p, nullptr);
    pool.dealloc(p, Layout{64, 1});
  }

  // After churn everything returned is cached at size 64.
  sfc::assert_eq(pool.free_bytes(), 64U);
}

}  // namespace sfc::alloc::test
