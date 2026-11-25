#include <Windows.h>

#include "sfc/sys/windows/ffi.h"

namespace sfc::sys::windows {

constexpr u32 kMaxName = 256U;

auto load_library(Str path) -> module_t {
  const auto os_path = FixedWStr<kMaxPath>::from_str(path);
  if (!os_path) {
    return nullptr;
  }

  const auto mod = ::LoadLibraryW(os_path);
  return mod;
}

void unload_library(module_t lib) {
  if (lib == nullptr) {
    return;
  }

  const auto mod = HMODULE(lib);
  (void)::FreeLibrary(mod);
}

auto get_symbol(module_t mod, Str name) -> symbol_t {
  if (mod == nullptr || name.is_empty()) {
    return nullptr;
  }

  const auto c_name = FixedCStr<kMaxName>::from_str(name);
  if (!c_name) {
    return nullptr;
  }

  const auto sym = ::GetProcAddress(HMODULE(mod), c_name);
  return sym;
}

}  // namespace sfc::sys::windows
