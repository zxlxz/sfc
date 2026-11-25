#pragma once

#include "sfc/alloc/list.h"

namespace sfc::collections {

class StrSet {
  struct V16B;

  List<V16B> _buf;
  StrSet(List<V16B> buf) noexcept;

 public:
  ~StrSet();
  StrSet(StrSet&&) noexcept;
  StrSet& operator=(StrSet&&) noexcept;

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
