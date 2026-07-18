#pragma once

#include "sfc/core.h"

namespace sfc::ffi {

class Library {
  void* _handle{nullptr};

 public:
  Library() noexcept;
  ~Library();
  Library(Library&& other) noexcept;
  Library& operator=(Library&& other) noexcept;

  static auto load(Str path) -> Library;

 public:
  auto get(Str name) const -> void*;
};

}  // namespace sfc::ffi
