#pragma once

#include "sfc/core/num.h"

namespace sfc::hash {

struct FNVHasher {
  static constexpr u64 OFFSET = 0xcbf29ce484222325ULL;
  static constexpr u64 PRIME = 0x100000001b3ULL;
  u64 _state = OFFSET;

 public:
  auto finish() const noexcept -> u64;

  void write_byte(u8 val) noexcept;
  void write_bytes(slice::Slice<const u8> bytes) noexcept;
};

using Hasher = FNVHasher;

}  // namespace sfc::hash
