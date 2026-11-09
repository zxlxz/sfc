#include "sfc/io/file.h"
#include "sfc/sys/io.h"

namespace sfc::io {

auto* File::Inn::operator->() {
  static_assert(sizeof(Inn) == sizeof(sys::File));
  static_assert(alignof(Inn) == alignof(sys::File));
  return (sys::File*)this;
}

File::File(RawFd fd) noexcept : _inn{fd} {}

File::~File() noexcept {
  if (_inn._fd == sys::kInvalidFd) {
    return;
  }
  _inn->close();
}

File::File(File&& other) noexcept : _inn{other._inn} {
  other._inn._fd = sys::kInvalidFd;
}

File& File::operator=(File&& other) noexcept {
  if (this != &other) {
    mem::swap(_inn, other._inn);
  }
  return *this;
}

auto File::from_raw_fd(RawFd fd) noexcept -> File {
  return File{fd};
}

auto File::as_raw_fd() const noexcept -> RawFd {
  return _inn._fd;
}

auto File::read(Slice<u8> buf) noexcept -> io::Result<usize> {
  return _inn->read(buf);
}

auto File::write(Slice<const u8> buf) noexcept -> io::Result<usize> {
  return _inn->write(buf);
}

auto File::flush() noexcept -> io::Result<> {
  return _inn->flush();
}

auto File::seek(SeekFrom pos) noexcept -> io::Result<u64> {
  return _inn->seek(pos);
}

}  // namespace sfc::io
