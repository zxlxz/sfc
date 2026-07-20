#include "sfc/ffi/library.h"
#include "sfc/ffi/os_str.h"

#define _SFC_SYS_FFI_
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
  auto os_path = ffi::OsString::from(path);
  if (!path.contains(".")) {
#ifdef _WIN32
    os_path.push_str(".dll");
#elif __APPLE__
    os_path.push_str(".dylib");
#else
    os_path.push_str(".so");
#endif
  }

  const auto handle = sys::load_library(os_path.as_ptr());
  auto res = Library{};
  res._handle = handle;
  return res;
}

auto Library::get(Str name) const -> Symbol {
  if (_handle == nullptr) {
    return nullptr;
  }

  const auto os_name = ffi::CString::from(name);
  const auto sym = sys::get_symbol(_handle, os_name.as_ptr());
  return Symbol(sym);
}

}  // namespace sfc::ffi
