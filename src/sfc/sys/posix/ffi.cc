#include <dlfcn.h>

#include "sfc/sys/posix/ffi.h"
#include "sfc/ffi/cstr.h"

namespace sfc::sys::posix {

using ffi::CString;

auto load_library(Str path) -> library_t {
  const auto os_path = CString::from_str(path);
  const auto lib_handle = ::dlopen(os_path.as_ptr(), RTLD_LAZY | RTLD_LOCAL);
  return lib_handle;
}

void unload_library(library_t lib) {
  if (lib == nullptr) {
    return;
  }

  ::dlclose(lib);
}

auto get_symbol(library_t lib, Str name) -> symbol_t {
  if (lib == nullptr || name.is_empty()) {
    return nullptr;
  }

  const auto os_name = CString::from_str(name);
  const auto sym_handle = ::dlsym(lib, os_name.as_ptr());
  return sym_handle;
}

}  // namespace sfc::sys::posix
