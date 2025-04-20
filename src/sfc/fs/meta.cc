
#include "meta.h"

#include "sfc/sys/io.h"
#include "sfc/ffi/cstring.h"

namespace sfc::fs {

namespace sys_imp = sys::io;

auto Meta::exists() const -> bool {
  return _attr != 0;
}

auto Meta::file_len() const -> u64 {
  return _size;
}

auto Meta::is_dir() const -> bool {
  const auto imp = sys_imp::FileAttr{_attr, 0};
  return imp.is_dir();
}

auto Meta::is_file() const -> bool {
  const auto imp = sys_imp::FileAttr{_attr, 0};
  return imp.is_file();
}

auto meta(const Path& path) -> io::Result<Meta> {
  const auto os_path = ffi::CString::from(path.as_str());
  const auto imp = sys_imp::lstat(os_path.c_str());
  if (!imp) {
    return io::Error::last_os_error();
  }

  const auto res = Meta{
      static_cast<u32>(imp._attr),
      static_cast<u64>(imp._size),
  };
  return res;
}

}  // namespace sfc::fs
