#include "sfc/io/file.h"
#include "sfc/sys/io.h"

namespace sfc::io {

File::File(RawFd fd) noexcept : _fd{fd} {}

File::~File() {
  auto imp = sys::File{_fd};
  (void)imp.close();

  _fd = InvalidFd;
}

File::File(File&& other) noexcept : _fd{other._fd} {
  other._fd = InvalidFd;
}

File& File::operator=(File&& other) noexcept {
  if (this != &other) {
    mem::swap(_fd, other._fd);
  }
  return *this;
}

auto File::from_raw_fd(RawFd fd) -> File {
  return File{fd};
}

auto File::as_raw_fd() const noexcept -> io::RawFd {
  return _fd;
}

auto File::read(Slice<u8> buf) noexcept -> io::Result<usize> {
  if (buf.is_empty()) {
    return {0UZ};
  }
  auto imp = sys::File{_fd};
  return imp.read(buf);
}

auto File::write(Slice<const u8> buf) noexcept -> io::Result<usize> {
  if (buf.is_empty()) {
    return {0UZ};
  }
  auto imp = sys::File{_fd};
  return imp.write(buf);
}

auto File::flush() -> io::Result<> {
  if (_fd == InvalidFd) {
    return Ok{};
  }
  auto imp = sys::File{_fd};
  return imp.flush();
}

auto File::seek(io::SeekFrom pos) noexcept -> io::Result<usize> {
  auto imp = sys::File{_fd};
  return imp.seek(pos);
}

}  // namespace sfc::io
