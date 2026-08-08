#pragma once

#include "sfc/alloc/list.h"
#include "sfc/sync/mutex.h"

namespace sfc::mem_pool {

using mem::Layout;

class Bucket;

class Pool {
 public:
  explicit Pool(usize cap) noexcept;
  virtual ~Pool() noexcept;

  static auto global() -> Pool&;

 public:
  auto total_bytes() const noexcept -> usize;
  auto free_bytes() const noexcept -> usize;

  auto alloc(usize size) -> void*;
  void dealloc(void* ptr, usize size);

 protected:
  auto bucket(usize size) -> Bucket&;
  auto find_oldest_bucket() -> Bucket&;

  auto fast_alloc(usize size) -> void*;
  void fast_dealloc(void* ptr, usize size);
  auto recycling(bool force, usize cap = 0) -> usize;

  virtual auto slow_alloc(Layout layout) -> void* = 0;
  virtual void slow_dealloc(void* ptr, Layout layout) = 0;

 private:
  const usize _cap;
  mutable sync::Mutex _mutex{};
  usize _seq{0};
  usize _total_bytes{0};
  usize _free_bytes{0};
  List<Bucket> _buckets{};
};

template <class A>
class XPool : public Pool {
  static constexpr auto kMaxFreeSize = usize{1} << 30;  // 1GB
  A _alloc{};

 public:
  explicit XPool(A a, usize cap = kMaxFreeSize) : Pool{cap}, _alloc{mem::move(a)} {}
  ~XPool() noexcept {}

 private:
  void* slow_alloc(Layout layout) override {
    return _alloc.allocate(layout);
  }

  void slow_dealloc(void* ptr, Layout layout) override {
    return _alloc.deallocate(ptr, layout);
  }
};

class Allocator {
  Pool* _pool{nullptr};

 public:
  Allocator(Pool& pool = Pool::global());
  ~Allocator();

 public:
  auto allocate(Layout layout) -> void*;
  void deallocate(void* ptr, Layout layout);
};

}  // namespace sfc::mem_pool
