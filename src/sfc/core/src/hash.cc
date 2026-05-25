#include "sfc/core/hash.h"
#include "sfc/core/slice.h"

namespace sfc::hash {

auto FNVHasher::finish() const noexcept -> u64 {
  return _state;
}

void FNVHasher::write_byte(u8 val) noexcept {
  _state ^= val;
  _state *= PRIME;
}

void FNVHasher::write_bytes(slice::Slice<const u8> bytes) noexcept {
  for (auto byte : bytes) {
    _state ^= byte;
    _state *= PRIME;
  }
}


}  // namespace sfc::hash
