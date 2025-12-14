#include "sfc/io/file.h"

#include "sfc/sys/io.h"

namespace sfc::io {

namespace sys_imp = sys::io;

File::File() noexcept : _fd{sys_imp::INVALID_FD} {}

File::~File() noexcept {
  if (_fd != sys_imp::INVALID_FD) {
    sys_imp::close(_fd);
  }
}

File::File(File&& other) noexcept : _fd{other._fd} {
  other._fd = sys_imp::INVALID_FD;
}

File& File::operator=(File&& other) noexcept {
  if (this != &other) {
    mem::swap(_fd, other._fd);
  }
  return *this;
}

auto File::from_fd(fd_t fd) noexcept -> File {
  auto res = File{};
  res._fd = fd;
  return res;
}

auto File::as_fd() const noexcept -> fd_t {
  return _fd;
}

auto File::is_open() const noexcept -> bool {
  return _fd != sys_imp::INVALID_FD;
}

auto File::read(Slice<u8> buf) noexcept -> Result<usize> {
  if (!this->is_open()) {
    return io::Error::InvalidInput;
  }
  if (buf.is_empty()) {
    return 0UL;
  }

  const auto res = sys_imp::read(_fd, buf.as_mut_ptr(), buf.len());
  if (res == -1) {
    return io::last_os_error();
  }
  return static_cast<usize>(res);
}

auto File::write(Slice<const u8> buf) noexcept -> Result<usize> {
  if (!this->is_open()) {
    return io::Error::InvalidInput;
  }
  if (buf.is_empty()) {
    return 0UL;
  }

  const auto res = sys_imp::write(_fd, buf.as_ptr(), buf.len());
  if (res == -1) {
    return io::last_os_error();
  }
  return static_cast<usize>(res);
}

}  // namespace sfc::io
