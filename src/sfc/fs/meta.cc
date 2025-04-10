
#include "meta.h"

#include "sfc/sys/io.h"

namespace sfc::fs {

namespace sys_imp = sys::io;

auto Meta::exists() const -> bool {
  return _mod != 0;
}

auto Meta::file_len() const -> u64 {
  return _len;
}

auto Meta::is_dir() const -> bool {
  const auto imp = sys_imp::FileAttr{_mod, 0};
  return imp.is_dir();
}

auto Meta::is_file() const -> bool {
  const auto imp = sys_imp::FileAttr{_mod, 0};
  return imp.is_file();
}

auto meta(const Path& path) -> io::Result<Meta> {
  const auto imp = sys_imp::lstat(path.c_str());
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
