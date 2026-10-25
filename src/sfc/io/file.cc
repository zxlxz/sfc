#include "sfc/io/file.h"
#include "sfc/sys/io.h"

namespace sfc::io {

File::File(RawFd fd) noexcept : _fd{fd} {}

File::~File() noexcept {
  if (_fd == sys::kInvalidFd) {
    return;
  }
  sys::File{_fd}.close();
}

File::File(File&& other) noexcept : _fd(other._fd) {
  other._fd = sys::kInvalidFd;
}

File& File::operator=(File&& other) noexcept {
  if (this != &other) {
    mem::swap(_fd, other._fd);
  }
  return *this;
}

auto File::from_raw_fd(RawFd fd) noexcept -> File {
  return File{fd};
}

auto File::as_raw_fd() const noexcept -> RawFd {
  return _fd;
}

auto File::read(Slice<u8> buf) noexcept -> io::Result<usize> {
  return sys::File{_fd}.read(buf);
}

auto File::write(Slice<const u8> buf) noexcept -> io::Result<usize> {
  return sys::File{_fd}.write(buf);
}

auto File::flush() noexcept -> io::Result<> {
  return sys::File{_fd}.flush();
}

auto File::seek(SeekFrom pos) noexcept -> io::Result<u64> {
  return sys::File{_fd}.seek(pos);
}

}  // namespace sfc::io
