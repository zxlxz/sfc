#include "sfc/ffi/library.h"
#include "sfc/ffi/os_str.h"

#define _SFC_SYS_LIBRARY_
#include "sfc/sys.h"

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
  if (this == &other) {
    return *this;
  }
  mem::swap(_handle, other._handle);
  return *this;
}

auto Library::load(Str path) -> Library {
  const auto os_path = ffi::OsString::from(path);

  const auto handle = sys::load_library(os_path.as_ptr());

  auto res = Library{};
  res._handle = handle;
  return res;
}

auto Library::get(Str name) const -> void* {
  if (_handle == nullptr) {
    return nullptr;
  }

  const auto os_name = ffi::CString::from(name);
  const auto sym = sys::get_symbol(_handle, os_name.as_ptr());
  return sym;
}

}  // namespace sfc::ffi
