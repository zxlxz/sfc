#include "file.h"

#include "sfc/sys/io.h"

namespace sfc::io {

namespace sys_imp = sys::io;

File::File() noexcept = default;

File::~File() {
  if (_fd == fd_t(-1)) {
    return;
  }
  sys_imp::File{_fd}.close();
}

File::File(File&& other) noexcept : _fd{other._fd} {
  other._fd = fd_t(-1);
}

auto File::from_raw_fd(fd_t fd) -> File {
  auto res = File{};
  res._fd = fd;
  return res;
}

File& File::operator=(File&& other) noexcept {
  auto tmp = static_cast<File&&>(*this);
  mem::swap(_fd, other._fd);
  return *this;
}

File::operator bool() const {
  return _fd != fd_t(-1);
}

auto File::read(Slice<u8> buf) -> Result<usize> {
  if (buf.is_empty()) {
    return 0UL;
  }

  auto imp = sys_imp::File{_fd};
  if (!imp) {
    return Error{ErrorKind::InvalidInput};
  }

  const auto res = imp.read(buf.as_mut_ptr(), buf.len());
  if (res == -1) {
    return io::Error::last_os_error();
  }
  return static_cast<u64>(res);
}

auto File::write(Slice<const u8> buf) -> Result<usize> {
  if (buf.is_empty()) {
    return 0UL;
  }

  auto imp = sys_imp::File{_fd};
  if (!imp) {
    return Error{ErrorKind::InvalidInput};
  }

  // do write
  const auto res = imp.write(buf.as_ptr(), buf.len());
  if (res == -1) {
    return io::Error::last_os_error();
  }
  return static_cast<u64>(res);
}

auto File::read_all(Vec<u8>& buf, usize buf_len) -> Result<usize> {
  const auto old_len = buf.len();
  while (true) {
    buf.reserve(buf_len);
    const auto ret = this->read({buf.as_mut_ptr() + buf.len(), buf_len});
    if (ret.is_err()) {
      return ret.unwrap_err();
    }
    
    const usize cnt = ret.unwrap();
    buf.set_len(buf.len() + cnt);
    if (cnt == 0) {
      break;
    }
  }

  const usize nread = buf.len() - old_len;
  return nread;
}

auto File::read_to_string(String& buf) -> Result<usize> {
  auto& v = buf.as_mut_vec();
  return this->read_all(*ptr::cast<Vec<u8>>(&v));
}

auto File::write_all(Slice<const u8> buf) -> Result<usize> {
  const auto old_len = buf.len();

  while (!buf.is_empty()) {
    const auto ret = this->write(buf);
    if (ret.is_err()) {
      return ret.unwrap_err();
    }
    const usize cnt = ret.unwrap();
    if (cnt == 0) {
      break;
    }
    buf = buf[{cnt, _}];
  }
  const usize nwrite = old_len - buf.len();
  return nwrite;
}

auto File::write_str(Str str) -> Result<usize> {
  return this->write_all(str.as_bytes());
}

}  // namespace sfc::io
