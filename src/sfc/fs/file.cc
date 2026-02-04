#include "sfc/fs/file.h"

#include "sfc/ffi/c_str.h"
#include "sfc/sys/fs.h"

namespace sfc::fs {

namespace sys_imp = sys::fs;
using sys::OsStr;

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
  const auto os_path = OsStr::xnew(path.as_str());

  const auto fd = sys_imp::open(os_path.ptr(), *this);
  if (fd == sys_imp::INVALID_FD) {
    return io::last_os_error();
  }

  auto res = File{};
  res._inn = io::File{fd};
  return res;
}

auto read(Path path) noexcept -> io::Result<Vec<u8>> {
  auto file = _TRY(File::open(path));
  auto buf = Vec<u8>{};
  _TRY(file.read_to_end(buf));
  return buf;
}

auto write(Path path, Slice<const u8> buf) noexcept -> io::Result<> {
  auto file = _TRY(File::create(path));
  _TRY(file.write_all(buf));
  return {};
}

}  // namespace sfc::fs
