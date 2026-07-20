#pragma once

#include "sfc/core.h"

namespace sfc::ffi {

class Library {
  using Symbol = void (*)();
  void* _handle{nullptr};

 public:
  Library() noexcept;
  ~Library();
  Library(Library&& other) noexcept;
  Library& operator=(Library&& other) noexcept;

  static auto load(Str path) -> Library;

 public:
  auto get(Str name) const -> Symbol;

  template <class F>
  auto get_func(Str name) const -> F {
    const auto sym = this->get(name);
    return F(sym);
  }
};

}  // namespace sfc::ffi
