#include "sfc/fs/file.h"

#include "sfc/ffi/os_str.h"
#include "sfc/sys/fs.h"

namespace sfc::fs {

namespace sys_imp = sys::fs;

auto File::open(Path path) noexcept -> io::Result<File> {
  const auto opts = OpenOptions{.read = true, .write = true};
  return opts.open(path);
}

auto File::create(Path path) noexcept -> io::Result<File> {
  const auto opts = OpenOptions{
      .create = true,
      .write = true,
      .truncate = true,
  };
  return opts.open(path);
}

auto File::read(Slice<u8> buf) noexcept -> io::Result<usize> {
  return _inn.read(buf);
}

auto File::write(Slice<const u8> buf) noexcept -> io::Result<usize> {
  return _inn.write(buf);
}

auto OpenOptions::open(Path path) const noexcept -> io::Result<File> {
  const auto os_path = ffi::OsString::from(path.as_str());

  const auto fd = sys_imp::open(os_path.ptr(), *this);
  if (fd == sys_imp::INVALID_FD) {
    return io::last_os_error();
  }

  auto res = File{};
  res._inn = io::File{fd};
  return res;
}

auto read(Path path) noexcept -> io::Result<Vec<u8>> {
  auto file = File::open(path);
  if (file.is_err()) {
    return ~file;
  }

  auto buf = Vec<u8>{};
  if (auto read_res = file->read_to_end(buf); read_res.is_err()) {
    return ~read_res;
  }
  return buf;
}

auto write(Path path, Slice<const u8> buf) noexcept -> io::Result<> {
  auto file = File::create(path);
  if (file.is_err()) {
    return ~file;
  }

  if (auto write_res = file->write_all(buf); write_res.is_err()) {
    return ~write_res;
  }
  return {};
}

}  // namespace sfc::fs
