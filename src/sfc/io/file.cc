#include "file.h"

#include "sfc/sys/io.inl"

namespace sfc::io {

namespace sys_imp = sys::io;

struct File::Inn : sys_imp::File {
  Inn(auto fd) noexcept : sys_imp::File{fd} {}
};

File::File() noexcept = default;

template <>
File::File(sys_imp::File inn) noexcept : _inn{Box<Inn>::xnew(inn)} {}

File::File(File&& other) noexcept = default;

File::~File() {
  if (!_inn) {
    return;
  }

  _inn->close();
}

File& File::operator=(File&& other) noexcept = default;

File::operator bool() const {
  return bool(_inn);
}

auto File::read(Slice<u8> buf) -> usize {
  assert_fmt(*this, "File::read: invalid file");
  if (buf.is_empty()) {
    return 0;
  }

  const auto res = _inn->read(buf);
  if (res == -1) {
    throw io::Error::last_os_error();
  }
  return static_cast<u64>(res);
}

auto File::write(Slice<const u8> buf) -> usize {
  assert_fmt(*this, "File::write: invalid file");
  if (buf.is_empty()) {
    return 0;
  }

  // do write
  const auto res = _inn->write(buf);
  if (res == -1) {
    throw io::Error::last_os_error();
  }
  return static_cast<u64>(res);
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
