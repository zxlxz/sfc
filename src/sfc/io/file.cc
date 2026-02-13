#include "sfc/io/file.h"

#include "sfc/sys/io.h"

namespace sfc::io {

namespace sys_imp = sys::io;

File::File() noexcept : _raw{sys_imp::INVALID_FD} {}

File::File(fd_t fd) noexcept : _raw{fd} {}

File::~File() noexcept {
  if (_raw != sys_imp::INVALID_FD) {
    sys_imp::close(_raw);
  }
}

File::File(File&& other) noexcept : _raw{other._raw} {
  other._raw = sys_imp::INVALID_FD;
}

File& File::operator=(File&& other) noexcept {
  if (this != &other) {
    mem::swap(_raw, other._raw);
  }
  return *this;
}

auto File::as_raw_fd() const noexcept -> fd_t {
  return _raw;
}

auto File::read(Slice<u8> buf) noexcept -> Result<usize> {
  if (_raw == sys_imp::INVALID_FD) {
    return io::Error::InvalidInput;
  }
  if (buf.is_empty()) {
    return 0UL;
  }

  const auto res = sys_imp::read(_raw, buf.as_mut_ptr(), buf.len());
  if (res == -1) {
    return io::last_os_error();
  }
  return static_cast<usize>(res);
}

auto File::write(Slice<const u8> buf) noexcept -> Result<usize> {
  if (_raw == sys_imp::INVALID_FD) {
    return io::Error::InvalidInput;
  }
  if (buf.is_empty()) {
    return 0UL;
  }

  const auto res = sys_imp::write(_raw, buf.as_ptr(), buf.len());
  if (res == -1) {
    return io::last_os_error();
  }
  return static_cast<usize>(res);
}

auto last_os_error() noexcept -> Error {
  const auto os_err = sys_imp::get_err();
  const auto io_err = sys_imp::map_err<Error>(os_err);
  return io_err;
}

}  // namespace sfc::io
