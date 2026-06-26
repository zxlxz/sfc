#include "sfc/core/hash.h"
#include "sfc/core/slice.h"

namespace sfc::hash {

auto FNVHasher::finish() const noexcept -> u64 {
  return _state;
}

void FNVHasher::write(slice::Slice<const u8> bytes) noexcept {
  for (auto byte : bytes) {
    _state ^= byte;
    _state *= PRIME;
  }
}

void FNVHasher::write_u8(u8 val) noexcept {
  this->write(mem::as_bytes(val));
}

void FNVHasher::write_u16(u16 val) noexcept {
  this->write(mem::as_bytes(val));
}

void FNVHasher::write_u32(u32 val) noexcept {
  this->write(mem::as_bytes(val));
}

void FNVHasher::write_u64(u64 val) noexcept {
  this->write(mem::as_bytes(val));
}

}  // namespace sfc::hash
