#pragma once

#include "sfc/core/num.h"
#include "sfc/core/mem.h"

namespace sfc::hash {

struct FNVHasher {
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

using Hasher = FNVHasher;

struct Hash {
  template <class T>
  static auto hash(const T& val) noexcept -> u64 {
    if constexpr (requires { val.hash(); }) {
      return val.hash();
    } else if constexpr (trait::int_<T>) {
      auto hasher = Hasher{};
      hasher.write(mem::as_bytes(val));
      return hasher.finish();
    } else {
      static_assert(false, "Hash::hash: cannot hash value type");
    }
  }
};

}  // namespace sfc::hash

namespace sfc {
using hash::Hash;
using hash::Hasher;
}  // namespace sfc
