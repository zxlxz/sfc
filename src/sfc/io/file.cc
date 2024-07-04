#include "file.h"

#include <unistd.h>

namespace sfc::io {

File::File() noexcept = default;

File::File(int fd) noexcept : _fd{fd} {}

File::File(File&& other) noexcept : _fd{other._fd} {
  other._fd = -1;
}

File::~File() {
  if (!*this) {
    return;
  }
  ::close(_fd);
}

File& File::operator=(File&& other) noexcept = default;

auto File::from_raw(int fd) -> File {
  return File{fd};
}

File::operator bool() const {
  return _fd != -1;
}

auto File::read(Slice<u8> buf) -> usize {
  assert_fmt(*this, "File::read: invalid file");
  if (buf.is_empty()) {
    return 0;
  }

  const auto ret = ::read(_fd, buf.as_mut_ptr(), buf.len());
  if (ret == -1) {
    throw Error::last_os_error();
  }
  return static_cast<usize>(ret);
}

auto File::write(Slice<const u8> buf) -> usize {
  assert_fmt(*this, "File::write: invalid file");
  if (buf.is_empty()) {
    return 0;
  }

  // do write
  const auto ret = ::write(_fd, buf.as_ptr(), buf.len());
  if (ret == -1) {
    throw Error::last_os_error();
  }
  return static_cast<usize>(ret);
}

auto File::read_all(Vec<u8>& buf, usize buf_len) -> usize {
  assert_fmt(*this, "File::read_all: invalid file");

  const auto old_len = buf.len();
  while (true) {
    buf.reserve(buf_len);
    const auto cnt = this->read({buf.as_mut_ptr() + buf.len(), buf_len});
    buf.set_len(buf.len() + cnt);
    if (cnt == 0) {
      break;
    }
  }
  const auto new_len = buf.len();
  return new_len - old_len;
}

auto File::read_to_string(String& buf) -> usize {
  assert_fmt(*this, "File::read_to_string: invalid file");

  auto& v = buf.as_mut_vec();
  return this->read_all(*ptr::cast<Vec<u8>>(&v));
}

auto File::write_all(Slice<const u8> buf) -> usize {
  assert_fmt(*this, "File::write_all: invalid file");

  const auto old_len = buf.len();

  while (!buf.is_empty()) {
    const auto ret = this->write(buf);
    if (ret == 0) {
      break;
    }
    buf = buf[{ret, _}];
  }
  const auto nwrite = old_len - buf.len();
  return nwrite;
}

auto File::write_str(Str str) -> usize {
  assert_fmt(*this, "File::write_str: invalid file");

  const auto ret = this->write_all(str.as_bytes());
  return ret;
}

}  // namespace sfc::io
