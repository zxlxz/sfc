#pragma once

#include "sfc/core/num.h"

namespace sfc::hash {

struct Hasher {
  template <class T>
  void write_any(this auto& self, const T& val) {
    if constexpr (requires { val.hash(self); }) {
      val.hash(self);
    } else if (trait::int_<T>) {
      if constexpr (sizeof(T) == 1) {
        self.write_u8(static_cast<u8>(val));
      } else if constexpr (sizeof(T) == 2) {
        self.write_u16(static_cast<u16>(val));
      } else if constexpr (sizeof(T) == 4) {
        self.write_u32(static_cast<u32>(val));
      } else if constexpr (sizeof(T) == 8) {
        self.write_u64(static_cast<u64>(val));
      }
    }
  }
};

struct FNVHasher : Hasher {
  static constexpr u64 OFFSET = 0xcbf29ce484222325ULL;
  static constexpr u64 PRIME = 0x100000001b3ULL;
  u64 _state = OFFSET;

 public:
  auto finish() const noexcept -> u64;
  void write(slice::Slice<const u8> bytes) noexcept;

  void write_u8(u8 val) noexcept;
  void write_u16(u16 val) noexcept;
  void write_u32(u32 val) noexcept;
  void write_u64(u64 val) noexcept;
};

using DefaultHasher = FNVHasher;

}  // namespace sfc::hash

namespace sfc {
using hash::DefaultHasher;
}  // namespace sfc
