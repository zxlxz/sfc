#include <Windows.h>
#undef min
#undef max

#include "sfc/sys/windows/ffi.h"

namespace sfc::sys::windows {

auto load_library(const wchar_t* path) -> void* {
  const auto mod = ::LoadLibraryW(path);
  return mod;
}

void unload_library(void* handle) {
  if (handle == nullptr) {
    return;
  }

  const auto mod = HMODULE(handle);
  (void)::FreeLibrary(mod);
}

auto get_symbol(void* handle, const char* name) -> isize(*)() {
  if (handle == nullptr || name == nullptr) {
    return nullptr;
  }

  const auto mod = HMODULE(handle);
  const auto sym = ::GetProcAddress(mod, name);
  return sym;
}

}  // namespace sfc::sys::windows
