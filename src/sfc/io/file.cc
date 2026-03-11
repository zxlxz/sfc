#if defined(__unix__) || defined(__APPLE__)
#include "sfc/sys/unix/io.inl"
#elif defined(_WIN32)
#include "sfc/sys/windows/io.inl"
#endif

#define _SFC_SYS_IO_
#include "sfc/io/file.h"

namespace sfc::io {

File::File(sys::File inn) noexcept : _inn{inn} {}

File::~File() noexcept {
  if (!_inn.is_valid()) {
    return;
  }
  _inn.close();
}

File::File(File&& other) noexcept : _inn{other._inn} {
  other._inn = {};
}

File& File::operator=(File&& other) noexcept {
  if (this != &other) {
    mem::swap(_inn, other._inn);
  }
  return *this;
}

auto File::as_raw_fd() const noexcept -> sys::RawFd {
  return _inn._fd;
}

auto File::is_valid() const noexcept -> bool {
  return _inn.is_valid();
}

auto File::read(Slice<u8> buf) noexcept -> Result<usize> {
  if (buf.is_empty()) {
    return 0UL;
  }

  const auto res = _inn.read(buf.as_mut_ptr(), buf.len());
  if (res == -1) {
    return io::last_os_error();
  }
  return static_cast<usize>(res);
}

auto File::write(Slice<const u8> buf) noexcept -> Result<usize> {
  if (buf.is_empty()) {
    return 0UL;
  }

  const auto res = _inn.write(buf.as_ptr(), buf.len());
  if (res == -1) {
    return io::last_os_error();
  }
  return static_cast<usize>(res);
}

auto File::seek(SeekFrom pos) noexcept -> Result<usize> {
  const auto where = static_cast<u32>(pos.whence);
  const auto res = _inn.seek(pos.offset, where);
  if (res == -1) {
    return io::last_os_error();
  }
  return static_cast<usize>(res);
}

auto File::flush() noexcept -> Result<> {
  if (!_inn.flush()) {
    return io::last_os_error();
  }
  return {};
}

auto File::is_tty() const noexcept -> bool {
  return _inn.is_tty();
}

}  // namespace sfc::io
