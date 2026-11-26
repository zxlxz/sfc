#include "sfc/io/file.h"
#include "sfc/sys/io.h"

namespace sfc::io {

File::File(RawFd fd) noexcept : _fd{fd} {}

File::~File() noexcept {
  // not owns fd, so don't close it here
}

File::File(File&& other) noexcept : _fd{other._fd} {
  other._fd = kInvalidFd;
}

File& File::operator=(File&& other) noexcept {
  if (this != &other) {
    mem::swap(_fd, other._fd);
  }
  return *this;
}

auto File::close() noexcept -> io::Result<> {
  if (_fd == kInvalidFd) {
    return Ok{};
  }

  _TRY(sys::close(_fd));
  _fd = kInvalidFd;
  return Ok{};
}

auto File::read(Slice<u8> buf) noexcept -> io::Result<usize> {
  return sys::read(_fd, buf);
}

auto File::write(Slice<const u8> buf) noexcept -> io::Result<usize> {
  return sys::write(_fd, buf);
}

auto File::flush() noexcept -> io::Result<> {
  return sys::flush(_fd);
}

auto File::seek(SeekFrom pos) noexcept -> io::Result<u64> {
  return sys::seek(_fd, pos);
}

auto File::is_terminal() const noexcept -> bool {
  return sys::is_console(_fd);
}

}  // namespace sfc::io
