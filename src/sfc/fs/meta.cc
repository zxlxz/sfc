
#include "meta.h"

#include "sfc/sys/fs.inl"

namespace sfc::fs {

namespace sys_imp = sys::fs;

auto Meta::exists() const -> bool {
  return _mod != 0;
}

auto Meta::file_len() const -> u64 {
  return _len;
}

auto Meta::is_dir() const -> bool {
  const auto imp = sys_imp::FileType{_mod, 0};
  return imp.is_dir();
}

auto Meta::is_file() const -> bool {
  const auto imp = sys_imp::FileType{_mod, 0};
  return imp.is_file();
}

auto Meta::is_link() const -> bool {
  const auto imp = sys_imp::FileType{_mod, 0};
  return imp.is_link();
}

auto meta(Path path) -> Meta {
  const auto os_path = ffi::CString::from(path.as_str());
  const auto imp = sys_imp::lstat(os_path);
  return Meta{
      static_cast<u32>(imp._mode),
      static_cast<u64>(imp._size),
  };
}

}  // namespace sfc::fs
