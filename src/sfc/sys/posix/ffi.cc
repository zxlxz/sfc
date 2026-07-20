#include <dlfcn.h>

namespace sfc::sys::posix {

auto load_library(const char* path) -> void* {
  const auto handle = ::dlopen(path, RTLD_LAZY | RTLD_LOCAL);
  return handle;
}

void unload_library(void* handle) {
  if (handle == nullptr) {
    return;
  }
  (void)::dlclose(handle);
}

auto get_symbol(void* handle, const char* name) -> void* {
  if (handle == nullptr || name == nullptr) {
    return nullptr;
  }
  const auto sym = ::dlsym(handle, name);
  return sym;
}

}  // namespace sfc::sys::posix
