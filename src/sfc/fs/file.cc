#include "sfc/fs/file.h"
#include "sfc/ffi/os_str.h"
#include "sfc/sys/fs.h"

namespace sfc::fs {

auto Metadata::exists() const noexcept -> bool {
  return _attr != 0;
}

auto Metadata::file_len() const noexcept -> u64 {
  return _size;
}

auto Metadata::is_dir() const noexcept -> bool {
  const auto imp = sys::FileAttr{_attr};
  return imp.is_dir();
}

auto Metadata::is_file() const noexcept -> bool {
  const auto imp = sys::FileAttr{_attr};
  return imp.is_file();
}

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

auto File::as_raw_fd() const noexcept -> sys::RawFd {
  return _inn.as_raw_fd();
}

auto File::is_valid() const -> bool {
  return _inn.is_valid();
}

auto File::read(Slice<u8> buf) noexcept -> io::Result<usize> {
  if (buf.is_empty()) {
    return {0UZ};
  }
  return _inn.read(buf);
}

auto File::write(Slice<const u8> buf) noexcept -> io::Result<usize> {
  if (buf.is_empty()) {
    return {0UZ};
  }
  return _inn.write(buf);
}

auto File::flush() -> io::Result<> {
  if (!_inn.is_valid()) {
    return Ok{};
  }
  return _inn.flush();
}

auto File::seek(io::SeekFrom pos) noexcept -> io::Result<usize> {
  return _inn.seek(pos);
}

auto File::metadata() noexcept -> io::Result<fs::Metadata> {
  const auto fd = _inn.as_raw_fd();
  return sys::fstat(fd);
}

auto OpenOptions::open(Path path) const noexcept -> io::Result<File> {
  const auto os_path = ffi::OsString::from(path.as_str());
  const auto fd = _TRY(sys::open(os_path.as_ptr(), *this));
  return {File::from_raw_fd(fd)};
}

auto read(Path path) noexcept -> io::Result<List<u8>> {
  auto file = _TRY(File::open(path));
  auto buf = List<u8>{};
  _TRY(file.read_to_end(buf));
  return {mem::move(buf)};
}

auto write(Path path, Slice<const u8> buf) noexcept -> io::Result<> {
  auto file = _TRY(File::create(path));
  _TRY(file.write_all(buf));
  return Ok{};
}

auto create_dir(Path path) -> io::Result<> {
  if (path._inn.is_empty() || path.is_root()) {
    return {io::Error::InvalidInput};
  }

  const auto os_path = ffi::OsString::from(path.as_str());
  return sys::mkdir(os_path.as_ptr());
}

auto create_dir_all(Path path) -> io::Result<> {
  const auto err = fs::create_dir(path).err();
  if (!err || *err == io::Error::AlreadyExists) {
    return Ok{};
  }

  const auto parent = path.parent();
  _TRY(fs::create_dir_all(parent));

  return fs::create_dir(path);
}

auto remove_dir(Path path) -> io::Result<> {
  const auto os_path = ffi::OsString::from(path.as_str());
  return sys::rmdir(os_path.as_ptr());
}

auto remove_file(Path path) -> io::Result<> {
  const auto os_path = ffi::OsString::from(path.as_str());
  return sys::unlink(os_path.as_ptr());
}

auto rename(Path old_path, Path new_path) -> io::Result<> {
  const auto os_old = ffi::OsString::from(old_path.as_str());
  const auto os_new = ffi::OsString::from(new_path.as_str());
  return sys::rename(os_old.as_ptr(), os_new.as_ptr());
}

auto metadata(Path path) -> io::Result<Metadata> {
  const auto os_path = ffi::OsString::from(path.as_str());
  const auto meta = _TRY(sys::lstat(os_path.as_ptr()));
  return {meta};
}

}  // namespace sfc::fs

namespace sfc::io {
template auto Read::read_exact(this fs::File&, Slice<u8>) -> Result<>;
template auto Read::read_to_end(this fs::File&, List<u8>&) -> Result<usize>;
template auto Read::read_to_string(this fs::File&, String&) -> Result<usize>;

template auto Write::write_all(this fs::File&, Slice<const u8>) -> Result<>;
template auto Write::write_str(this fs::File&, Str) -> Result<>;
}  // namespace sfc::io
