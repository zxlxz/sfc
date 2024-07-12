
#include "meta.h"

#include "sfc/ffi.h"
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

auto Meta::is_symlink() const -> bool {
  const auto imp = sys_imp::FileType{_mod, 0};
  return imp.is_symlink();
}

auto meta(Path path) -> io::Result<Meta> {
  const auto os_path = ffi::CString::from(path.as_str());
  const auto imp_ret = sys_imp::lstat(os_path);
  if (!imp_ret) {
    return io::Error::last_os_error();
  }

  const auto imp = imp_ret.unwrap();
  const auto res = Meta{
      static_cast<u32>(imp._mode),
      static_cast<u64>(imp._size),
  };
  return res;
}

}  // namespace sfc::fs
