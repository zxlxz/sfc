#include "sfc/fs/file.h"

#include "sfc/sys/fs.h"

namespace sfc::fs {

namespace sys_imp = sys::fs;

File::File(io::File&& inn) noexcept : _inn{static_cast<io::File&&>(inn)} {}

File::~File() noexcept = default;

File::File(File&&) noexcept = default;

File& File::operator=(File&&) noexcept = default;

auto File::open(const Path& path) -> io::Result<File> {
  const auto opts = OpenOptions{.read = true, .write = true};
  return opts.open(path);
}

auto File::create(const Path& path) -> io::Result<File> {
  const auto opts = OpenOptions{.create = true, .write = true, .truncate = true};
  return opts.open(path);
}

auto File::read(Slice<u8> buf) -> io::Result<usize> {
  return _inn.read(buf);
}

auto File::read_all(Vec<u8>& buf, usize buf_len) -> io::Result<usize> {
  return _inn.read_all(buf, buf_len);
}

auto File::write(Slice<const u8> buf) -> io::Result<usize> {
  return _inn.write(buf);
}

auto File::write_all(Slice<const u8> buf) -> io::Result<usize> {
  return _inn.write_all(buf);
}

auto File::read_to_string(String& buf, usize buf_len) -> io::Result<usize> {
  return _inn.read_all(buf.as_mut_vec(), buf_len);
}

auto File::write_str(Str str) -> io::Result<usize> {
  return _inn.write_all(str.as_bytes());
}

auto OpenOptions::open(const Path& path) const -> io::Result<File> {
  const auto c_path = CString::from(path.as_str());

  const auto fd = sys_imp::open(c_path, *this);
  if (fd == sys_imp::INVALID_FD) {
    return io::Error::last_os_error();
  }

  return File{io::File{fd}};
}

}  // namespace sfc::fs
