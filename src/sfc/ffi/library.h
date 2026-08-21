#pragma once

#include "sfc/core.h"

namespace sfc::ffi {

struct Func {
  void (*_func)();

 public:
  template <class F>
  operator F() const {
    return reinterpret_cast<F>(_func);
  }
};

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
  auto func(Str name) const -> Func;
};

}  // namespace sfc::ffi
