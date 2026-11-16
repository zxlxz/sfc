#include "sfc/ffi/library.h"
#include "sfc/sys/ffi.h"

namespace sfc::ffi {

Library::Library() noexcept {}

Library::~Library() {
  if (_handle == nullptr) {
    return;
  }
  sys::unload_library(_handle);
}

Library::Library(Library&& other) noexcept : _handle{mem::take(other._handle)} {}

auto Library::operator=(Library&& other) noexcept -> Library& {
  if (this != &other) {
    mem::swap(_handle, other._handle);
  }
  return *this;
}

auto Library::load(Str path) -> Option<Library> {
  if (path.is_empty()) {
    return {};
  }

  const auto lib = sys::load_library(path);
  if (lib == nullptr) {
    return {};
  }

  auto res = Library{};
  res._handle = lib;
  return res;
}

auto Library::func(Str name) const -> Func {
  if (_handle == nullptr) {
    return Func{nullptr};
  }

  const auto sym = sys::get_symbol(_handle, name);
  const auto sym_fun = ptr::cast<void()>(sym);
  return Func{sym_fun};
}

}  // namespace sfc::ffi
