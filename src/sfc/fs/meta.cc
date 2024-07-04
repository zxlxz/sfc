
#include "meta.h"

#include <sys/stat.h>

namespace sfc::fs {

auto Meta::exists() const -> bool {
  return _mod != 0;
}

auto Meta::file_len() const -> u64 {
  return _len;
}

auto Meta::is_dir() const -> bool {
  const auto res = S_ISDIR(_mod);
  return res;
}

auto Meta::is_file() const -> bool {
  const auto res = S_ISREG(_mod);
  return res;
}

auto Meta::is_link() const -> bool {
  const auto res = S_ISLNK(_mod);
  return res;
}

auto meta(Path path) -> Meta {
  struct stat st;
  const auto ret = ::lstat(path.to_c_string(), &st);
  if (ret == -1) {
    return {0, 0};
  }

  const auto res = Meta{
      static_cast<u64>(st.st_size),
      static_cast<u32>(st.st_mode),
  };
  return res;
}

}  // namespace sfc::fs
