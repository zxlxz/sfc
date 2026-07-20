#include "sfc/ffi/library.h"
#include "sfc/ffi/os_str.h"

#define _SFC_SYS_FFI_
#include "sfc/sys.h"

namespace sfc::ffi {

static auto make_lib_path(Str name) -> String {
  if (name.contains('/')) {  // this is a path, not a library name
    return String::from(name);
  }
  if (name.contains('.')) {  // already has an extension, use it as is
    return String::from(name);
  }

#ifdef _WIN32
  return string::format("{}.dll", name);
#elif defined(__APPLE__)
  return string::format("lib{}.dylib", name);
#else
  return string::format("lib{}.so", name);
#endif
}

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
  const auto lib_path = ffi::make_lib_path(path);

  const auto os_path = ffi::OsString::from(lib_path.as_str());
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
