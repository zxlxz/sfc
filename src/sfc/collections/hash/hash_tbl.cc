#include "sfc/collections/hash/hash_tbl.h"

namespace sfc::collections::hash {

static auto adjust_capacity(usize n) -> usize {
  auto t = usize{8};
  while (t < n) {
    t <<= 1;
  }
  return t;
}

RawTbl::RawTbl() {}

RawTbl::~RawTbl() {
  if (_ptr == nullptr) {
    return;
  }

  // hack trick: retrieve element size from the first byte of the control block
  const auto element_size = _ptr[_cap];
  const auto ctrl_size = num::align_up(_cap, kAlign);
  const auto data_size = _cap * element_size;
  const auto layout = mem::Layout{ctrl_size + data_size, kAlign};
  _alloc.deallocate(_ptr, layout);
}

RawTbl::RawTbl(RawTbl&& other) noexcept
    : _ptr{mem::take(other._ptr)}, _cap{mem::take(other._cap)}, _alloc{mem::move(other._alloc)} {}

RawTbl& RawTbl::operator=(RawTbl&& other) noexcept {
  if (this != &other) {
    mem::swap(_ptr, other._ptr);
    mem::swap(_cap, other._cap);
    mem::swap(_alloc, other._alloc);
  }
  return *this;
}

auto RawTbl::with_capacity(usize min_cap, usize element_size) -> RawTbl {
  if (min_cap == 0) {
    return RawTbl{};
  }

  const auto capacity = adjust_capacity(min_cap);
  const auto ctrl_size = num::align_up(capacity, kAlign);
  const auto data_size = capacity * element_size;
  const auto layout = mem::Layout{ctrl_size + data_size, kAlign};

  auto a = alloc::Global{};
  auto res = RawTbl{};
  res._cap = capacity;
  res._ptr = ptr::cast<u8>(a.allocate(layout));
  res._alloc = mem::move(a);
  return res;
}

void RawTbl::init(usize element_size) {
  if (_ptr == nullptr) {
    return;
  }

  const auto ctrl_size = num::align_up(_cap, kAlign);
  ptr::write_bytes(_ptr, CTRL_NUL, ctrl_size);

  // hack trick: store element size in the first byte of the control block
  _ptr[_cap] = u8(element_size);
}

}  // namespace sfc::collections::hash
