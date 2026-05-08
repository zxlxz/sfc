#pragma once

#include "sfc/core/num.h"

namespace sfc::hash {

struct FNVHasher {
  static constexpr u64 OFFSET = 0xcbf29ce484222325ULL;
  static constexpr u64 PRIME = 0x100000001b3ULL;
  u64 _state = OFFSET;

 public:
  auto finish() const noexcept -> u64 {
    return _state;
  }

  void write_byte(u8 val) noexcept {
    _state ^= val;
    _state *= PRIME;
  }

  template <u32 N>
  void write_bytes(const u8 (&bytes)[N]) noexcept {
    for (auto byte : bytes) {
      _state ^= byte;
      _state *= PRIME;
    }
  }
};

using Hasher = FNVHasher;

}  // namespace sfc::hash
