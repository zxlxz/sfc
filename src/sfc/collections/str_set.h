#pragma once

#include "sfc/alloc/list.h"

namespace sfc::collections {

class StrSet {
  static constexpr usize kAlignSize = 8U;

  List<u8> _buf;
  StrSet(List<u8> buf) noexcept : _buf{mem::move(buf)} {}

 public:
  static auto new_() -> StrSet;
  static auto with_capacity(usize capacity) -> StrSet;

 public:
  void reserve(usize add_cnt);

  auto search(Str s) const -> Option<u32>;

  struct InsertResult {
    u32 idx;
    bool inserted;
  };

  /// Inserts the given string into the set.
  /// @note if `s` is empty, insert will panic
  auto insert(Str s) -> InsertResult;
};

}  // namespace sfc::collections
