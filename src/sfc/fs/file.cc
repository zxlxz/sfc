#if defined(__unix__) || defined(__APPLE__)
#include "sfc/sys/unix/fs.inl"
#elif defined(_WIN32)
#include "sfc/sys/windows/fs.inl"
#endif

#include "sfc/fs/file.h"
#include "sfc/ffi/os_str.h"

namespace sfc::fs {

File::File() noexcept : _inn{} {}

File::~File() noexcept {}

File::File(File&&) noexcept = default;
File& File::operator=(File&& other) noexcept = default;

auto File::from_raw_fd(sys::RawFd fd) -> File {
  auto res = File{};
  res._inn = sys::File{fd};
  return res;
}

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
  if (buf.is_empty()) {
    return 0UL;
  }
  return _inn.read(buf);
}

auto File::write(Slice<const u8> buf) noexcept -> io::Result<usize> {
  if (buf.is_empty()) {
    return 0UL;
  }
  return _inn.write(buf);
}

auto File::seek(io::SeekFrom pos) noexcept -> io::Result<usize> {
  const auto where = static_cast<u32>(pos.whence);
  return _inn.seek(pos.offset, where);
}

auto OpenOptions::open(Path path) const noexcept -> io::Result<File> {
  const auto os_path = ffi::OsString::from(path.as_str());

  const auto sys_opts = sys::OpenOptions{
      ._append = append,
      ._create = create,
      ._create_new = create_new,
      ._read = read,
      ._write = write,
      ._truncate = truncate,
  };

  const auto fd = _TRY(sys_opts.open(os_path.ptr()));
  return File::from_raw_fd(fd);
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
