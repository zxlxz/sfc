#pragma once

#include "sfc/core/trait.h"

namespace sfc::hash {

struct FNVHasher {
  static constexpr u64 OFFSET = 0xcbf29ce484222325ULL;
  static constexpr u64 PRIME = 0x100000001b3ULL;
  u64 _state = 0;

 public:
  auto finish() const noexcept -> u64 {
    return _state;
  }

  void write(u8 val) noexcept {
    _state ^= val;
    _state *= PRIME;
  }

  void write_int(trait::int_ auto val) noexcept {
    const auto p = reinterpret_cast<const u8*>(&val);
    for (auto i = 0U; i < sizeof(val); ++i) {
      _state ^= p[i];
      _state *= PRIME;
    }
  }
};

}  // namespace sfc::hash
