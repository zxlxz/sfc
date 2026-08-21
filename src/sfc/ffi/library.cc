#include "sfc/ffi/library.h"
#include "sfc/ffi/os_str.h"
#include "sfc/sys/ffi.h"

namespace sfc::ffi {

struct LibPathBuilder {
  Str prefix;
  Str suffix;

  auto build(Str name) const -> String {
    if (name.contains('/')) {  // this is a path, not a library name
      return String::from(name);
    }

    if (name.contains('.')) {  // already has an extension, use it as is
      return String::from(name);
    }

    auto res = String{};
    if (!name.starts_with(prefix)) res.push_str(prefix);
    res.push_str(name);
    if (!name.ends_with(suffix)) res.push_str(suffix);
    return res;
  }
};

static auto make_lib_path(Str name) -> String {
#ifdef _WIN32
  const auto builder = LibPathBuilder{{}, ".dll"};
#elif defined(__APPLE__)
  const auto builder = LibPathBuilder{"lib", ".dylib"};
#elif defined(__unix__)
  const auto builder = LibPathBuilder{"lib", ".so"};
#endif
  return builder.build(name);
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

auto Library::load(Str path) -> Option<Library> {
  if (path.is_empty()) {
    return {};
  }

  const auto lib_path = ffi::make_lib_path(path);
  const auto os_path = ffi::OsString::from(lib_path.as_str());
  const auto handle = sys::load_library(os_path.as_ptr());
  if (handle == nullptr) {
    return {};
  }

  auto res = Library{};
  res._handle = handle;
  return res;
}

auto Library::func(Str name) const -> Func {
  if (_handle == nullptr) {
    return Func{nullptr};
  }

  const auto os_name = ffi::CString::from(name);

  const auto sym = sys::get_symbol(_handle, os_name.as_ptr());
  return Func{(void (*)())sym};
}

}  // namespace sfc::ffi
