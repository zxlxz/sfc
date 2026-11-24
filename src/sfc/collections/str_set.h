#pragma once

#include "sfc/alloc/list.h"

namespace sfc::collections {

class StrSet {
  static constexpr u32 kBlockSize = sizeof(u64);

  List<u64> _buf;
  StrSet(List<u64> buf) noexcept;

 public:
  ~StrSet();
  StrSet(StrSet&&) noexcept = default;
  StrSet& operator=(StrSet&&) noexcept = default;

  static auto new_() -> StrSet;
  static auto with_capacity(usize capacity) -> StrSet;

 public:
  void reserve(usize add_cnt);

  auto search(Str s) const -> Option<u32>;

  struct InsertResult {
    u32 idx;
    bool inserted;
  };
  auto insert(Str s) -> InsertResult;
};

}  // namespace sfc::collections
