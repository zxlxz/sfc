#include "sfc/fs/file.h"

#include "sfc/sys/fs.h"

namespace sfc::fs {

namespace sys_imp = sys::fs;

File::File() noexcept = default;

File::~File() noexcept = default;

File::File(File&&) noexcept = default;

File& File::operator=(File&&) noexcept = default;

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
  const auto c_path = CString::from(path.as_str());

  auto file = io::File::from_fd(sys_imp::open(c_path, *this));
  if (!file.is_open()) {
    return io::last_os_error();
  }

  auto res = File{};
  res._inn = mem::move(file);
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
