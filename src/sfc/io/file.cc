#include "sfc/io/file.h"

#include "sfc/sys/io.h"

namespace sfc::io {

namespace sys_imp = sys::io;

File::File(fd_t fd) : _fd{fd} {}

File::~File() {
  if (_fd == sys_imp::INVALID_FD) {
    return;
  }

  sys_imp::close(_fd);
}

File::File(File&& other) noexcept : _fd{other._fd} {
  other._fd = sys_imp::INVALID_FD;
}

File& File::operator=(File&& other) noexcept {
  if (_fd != sys_imp::INVALID_FD) {
    sys_imp::close(_fd);
  }

  _fd = mem::replace(other._fd, sys_imp::INVALID_FD);
  return *this;
}

auto File::as_fd() const -> fd_t {
  return _fd;
}

File::operator bool() const {
  return _fd != sys_imp::INVALID_FD;
}

auto File::read(Slice<u8> buf) -> Result<usize> {
  if (buf.is_empty()) {
    return 0UL;
  }
  if (_fd == sys_imp::INVALID_FD) {
    return io::Error{io::ErrorKind::InvalidInput, 0};
  }

  const auto res = sys_imp::read(_fd, buf.as_mut_ptr(), buf.len());
  if (res == -1) {
    return io::Error::last_os_error();
  }
  return static_cast<usize>(res);
}

auto File::write(Slice<const u8> buf) -> Result<usize> {
  if (buf.is_empty()) {
    return 0UL;
  }
  if (_fd == sys_imp::INVALID_FD) {
    return io::Error{io::ErrorKind::InvalidInput, 0};
  }

  const auto res = sys_imp::write(_fd, buf.as_ptr(), buf.len());
  if (res == -1) {
    return io::Error::last_os_error();
  }
  return static_cast<usize>(res);
}

auto File::read_all(Vec<u8>& buf, usize buf_len) -> Result<usize> {
  if (_fd == sys_imp::INVALID_FD) {
    return io::Error{io::ErrorKind::InvalidInput, 0};
  }
  const auto old_len = buf.len();

  while (true) {
    buf.reserve(buf_len);
    const auto ret = this->read({buf.as_mut_ptr() + buf.len(), buf_len});
    if (ret.is_err()) {
      return ~ret;
    }

    const auto cnt = *ret;
    if (cnt == 0) {
      break;
    }
    buf.set_len(buf.len() + cnt);
  }

  const auto nread = buf.len() - old_len;
  return nread;
}

auto File::read_to_string(String& str, usize buf_len) -> Result<usize> {
  if (_fd == sys_imp::INVALID_FD) {
    return io::Error{io::ErrorKind::InvalidInput, 0};
  }

  return this->read_all(str.as_mut_vec(), buf_len);
}

auto File::write_all(Slice<const u8> buf) -> Result<usize> {
  if (_fd == sys_imp::INVALID_FD) {
    return io::Error{io::ErrorKind::InvalidInput, 0};
  }

  const auto old_len = buf.len();
  while (!buf.is_empty()) {
    const auto ret = this->write(buf);
    if (ret.is_err()) {
      return ~ret;
    }

    const auto cnt = *ret;
    if (cnt == 0) {
      break;
    }
    buf = buf[{cnt, _}];
  }
  const auto nwrite = old_len - buf.len();
  return nwrite;
}

auto File::write_str(Str str) -> Result<usize> {
  if (_fd == sys_imp::INVALID_FD) {
    return io::Error{io::ErrorKind::InvalidInput, 0};
  }

  return this->write_all(str.as_bytes());
}

}  // namespace sfc::io
