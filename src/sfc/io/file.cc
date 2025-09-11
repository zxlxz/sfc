#include "sfc/io/file.h"

#include "sfc/sys/io.h"

namespace sfc::io {

namespace sys_imp = sys::io;

File::File() noexcept : _fd{sys_imp::INVALID_FD} {}

File::~File() noexcept {
  this->close();
}

File::File(File&& other) noexcept : _fd{other._fd} {
  other._fd = sys_imp::INVALID_FD;
}

File& File::operator=(File&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  this->close();
  _fd = other._fd;
  other._fd = sys_imp::INVALID_FD;
  return *this;
}

auto File::from_fd(fd_t fd) -> File {
  auto res = File{};
  res._fd = fd;
  return res;
}

auto File::as_fd() const -> fd_t {
  return _fd;
}

auto File::is_open() const -> bool {
  return _fd != sys_imp::INVALID_FD;
}

void File::close() {
  if (!this->is_open()) {
    return;
  }
  sys_imp::close(_fd);
  _fd = sys_imp::INVALID_FD;
}

auto File::read(Slice<u8> buf) -> Result<usize> {
  if (buf.is_empty()) {
    return 0UL;
  }
  if (!this->is_open()) {
    return io::Error{ErrorKind::InvalidInput, 0};
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
  if (!this->is_open()) {
    return io::Error{ErrorKind::InvalidInput, 0};
  }

  const auto res = sys_imp::write(_fd, buf.as_ptr(), buf.len());
  if (res == -1) {
    return io::Error::last_os_error();
  }
  return static_cast<usize>(res);
}

auto File::read_to_end(Vec<u8>& buf, usize buf_len) -> Result<usize> {
  if (!this->is_open()) {
    return io::Error{ErrorKind::InvalidInput, 0};
  }
  const auto old_len = buf.len();

  while (true) {
    buf.reserve(buf_len);
    auto read_res = this->read({buf.as_mut_ptr() + buf.len(), buf_len});
    if (read_res.is_err()) {
      return mem::move(read_res).unwrap_err();
    }
    const auto cnt = mem::move(read_res).unwrap();
    if (cnt == 0) {
      break;
    }
    buf.set_len(buf.len() + cnt);
  }

  const auto nread = buf.len() - old_len;
  return nread;
}

auto File::read_to_string(String& buf, usize buf_len) -> Result<usize> {
  return this->read_to_end(buf.as_mut_vec(), buf_len);
}

auto File::write_all(Slice<const u8> buf) -> Result<usize> {
  if (!this->is_open()) {
    return io::Error{ErrorKind::InvalidInput, 0};
  }

  const auto old_len = buf.len();
  while (!buf.is_empty()) {
    auto write_res = this->write(buf);
    if (write_res.is_err()) {
      return mem::move(write_res).unwrap_err();
    }

    const auto cnt = mem::move(write_res).unwrap();
    if (cnt == 0) {
      break;
    }
    buf = buf[{cnt, _}];
  }
  const auto nwrite = old_len - buf.len();
  return nwrite;
}

auto File::write_str(Str buf) -> io::Result<usize> {
  return this->write_all(buf.as_bytes());
}
}  // namespace sfc::io
