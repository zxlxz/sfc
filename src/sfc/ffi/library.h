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

  static auto load(Str path) -> Option<Library>;

 public:
  auto symbol(Str name) const -> Symbol;

  template <class F>
  auto func(Str name) const -> Option<F> {
    const auto sym = this->symbol(name);
    if (sym == nullptr) {
      return {};
    }
    return F(sym);
  }
};

}  // namespace sfc::ffi
