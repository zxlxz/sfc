#include <dlfcn.h>

#include "sfc/sys/posix/ffi.h"

namespace sfc::sys::posix {

constexpr u32 kMaxName = 256U;

auto load_library(Str path) -> library_t {
  const auto os_path = FixedCStr<kMaxPath>::from_str(path);
  if (!os_path) {
    return nullptr;
  }

  const auto lib_handle = ::dlopen(os_path, RTLD_LAZY | RTLD_LOCAL);
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

  const auto os_name = FixedCStr<kMaxName>::from_str(name);
  if (!os_name) {
    return nullptr;
  }

  const auto sym_handle = ::dlsym(lib, os_name);
  return sym_handle;
}

}  // namespace sfc::sys::posix
