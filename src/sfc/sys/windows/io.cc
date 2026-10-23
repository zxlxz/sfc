#include <Windows.h>

#include "sfc/ffi/wstr.h"
#include "sfc/sys/windows/io.h"

namespace sfc::io {

File::File(RawFd fd) noexcept : _fd{fd} {}

File::~File() noexcept {
  if (_fd == nullptr || _fd == INVALID_HANDLE_VALUE) {
    return;
  }
  ::CloseHandle(_fd);
}

File::File(File&& other) noexcept : _fd(other._fd) {
  other._fd = nullptr;
}

File& File::operator=(File&& other) noexcept {
  if (this != &other) {
    mem::swap(_fd, other._fd);
  }
  return *this;
}

auto File::from_raw_fd(RawFd fd) noexcept -> File {
  return File{fd};
}

auto File::as_raw_fd() const noexcept -> RawFd {
  return _fd;
}

auto File::read(Slice<u8> buf) noexcept -> io::Result<usize> {
  const auto buf_ptr = buf._ptr;
  const auto buf_len = u32(cmp::min(buf._len, U32::MAX));

  auto nread = 0UL;
  if (!::ReadFile(_fd, buf_ptr, buf_len, &nread, nullptr)) {
    return io::last_os_error();
  }
  return usize{nread};
}

auto File::write(Slice<const u8> buf) noexcept -> io::Result<usize> {
  const auto buf_ptr = buf._ptr;
  const auto buf_len = u32(cmp::min(buf._len, U32::MAX));

  auto nwrite = 0UL;
  if (!::WriteFile(_fd, buf_ptr, buf_len, &nwrite, nullptr)) {
    return io::last_os_error();
  }
  return usize{nwrite};
}

auto File::flush() noexcept -> io::Result<> {
  const auto ret = ::FlushFileBuffers(_fd);
  if (!ret) {
    return io::last_os_error();
  }
  return Ok{};
}

auto File::seek(SeekFrom pos) noexcept -> io::Result<u64> {
  const auto move_method = [=] -> DWORD {
    switch (pos._tag) {
      case SeekFrom::Kind::Start:   return FILE_BEGIN;
      case SeekFrom::Kind::Current: return FILE_CURRENT;
      case SeekFrom::Kind::End:     return FILE_END;
    }
  }();

  const auto old_pos = LARGE_INTEGER{.QuadPart = pos.offset};
  auto new_pos = LARGE_INTEGER{};
  if (!::SetFilePointerEx(_fd, old_pos, &new_pos, move_method)) {
    return io::last_os_error();
  }

  const auto ret_pos = num::cast_unsigned(new_pos.QuadPart);
  return u64{ret_pos};
}

}  // namespace sfc::io

namespace sfc::sys::windows {

using chr::U16Chars;

template <class R>
struct WBufReader {
  R& _inn;

 public:
  auto read(Slice<u8> buf) -> io::Result<usize> {
    static constexpr auto kBufLen = 256U;
    u16 u16_buf[kBufLen] = {};

    auto rem = buf;
    while (!rem.is_empty()) {
      // read: utf16
      const auto u16_len = cmp::min((buf._len + 1) / 2, kBufLen);
      const auto nread = _TRY(_inn.read_u16({u16_buf, u16_len}));
      if (nread == 0) {
        break;
      }

      // convert: utf16 -> utf8
      chr::decode_utf16({u16_buf, nread}).for_each([&](char32_t ch) {
        const auto v = chr::encode_utf8(rem, ch);
        rem = Slice{rem._ptr + v._len, rem._len - v._len};
      });
    }

    return buf._len - rem._len;
  }
};

template <class W>
struct WBufWriter {
  static constexpr usize BUF_LEN = 256U;

  W& _inn;
  u16 _buf[BUF_LEN] = {};
  usize _len = 0;

 public:
  auto write(Slice<const u8> buf) -> io::Result<usize> {
    static constexpr auto kBufLen = 256U;

    auto rem = chr::decode_utf8(buf);
    while (!rem._buf.is_empty()) {
      u16 u16_buf[kBufLen] = {};

      // convert: utf8->utf16
      auto u16_len = usize{0UL};
      rem.try_for_each([&](char32_t ch) {
        const auto v = chr::encode_utf16({u16_buf + u16_len, 2}, ch);
        u16_len += v._len;
        return v._len > 0 && u16_len + 2 <= kBufLen;
      });

      // write: utf16
      _TRY(_inn.write_u16({u16_buf, u16_len}));
    }
    return buf._len;
  }

  auto flush() -> io::Result<> {
    _TRY(_inn.write_u16({_buf, _len}));
    _len = 0;
    return Ok{};
  }
};

auto StdIo::Input() -> StdIo {
  static auto file = File{::GetStdHandle(STD_INPUT_HANDLE)};
  return StdIo{file};
}

auto StdIo::Output() -> StdIo {
  static auto file = File{::GetStdHandle(STD_OUTPUT_HANDLE)};
  return StdIo{file};
}

auto StdIo::Error() -> StdIo {
  static auto file = File{::GetStdHandle(STD_ERROR_HANDLE)};
  return StdIo{file};
}

auto StdIo::is_console() const -> bool {
  auto mode = DWORD{};
  return ::GetConsoleMode(_file.as_raw_fd(), &mode);
}

auto StdIo::is_utf8_console() const -> bool {
  const auto code_page = ::GetConsoleCP();
  return code_page == CP_UTF8;
}

auto StdIo::flush() -> io::Result<> {
  return Ok{};
}

auto StdIo::read_u16(Slice<u16> buf) -> io::Result<usize> {
  const auto buf_ptr = buf._ptr;
  const auto buf_len = u32(cmp::min(buf._len, U32::MAX));

  auto nret = DWORD{};
  if (!::ReadConsoleW(_file.as_raw_fd(), buf_ptr, buf_len, &nret, nullptr)) {
    return io::last_os_error();
  }
  return usize{nret};
}

auto StdIo::write_u16(Slice<const u16> buf) -> io::Result<usize> {
  const auto buf_ptr = buf._ptr;
  const auto buf_len = u32(cmp::min(buf._len, U32::MAX));

  auto nret = DWORD{};
  if (!::WriteConsoleW(_file.as_raw_fd(), buf_ptr, buf_len, &nret, nullptr)) {
    return io::last_os_error();
  }

  return usize{buf._len};
}

auto StdIo::read(Slice<u8> buf) -> io::Result<usize> {
  if (this->is_utf8_console()) {
    return _file.read(buf);
  }

  auto rbuf = WBufReader{*this};
  return rbuf.read(buf);
}

auto StdIo::write(Slice<const u8> buf) -> io::Result<usize> {
  if (this->is_utf8_console()) {
    return _file.write(buf);
  }

  auto wbuf = WBufWriter{*this};
  return wbuf.write(buf);
}

auto os_error() -> u32 {
  auto err = ::GetLastError();
  return u32(err);
}

auto io_error(u32 code) -> io::Error {
  switch (code) {
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:    return io::Error::NotFound;
    case ERROR_ACCESS_DENIED:     return io::Error::PermissionDenied;
    case WSAECONNREFUSED:         return io::Error::ConnectionRefused;
    case WSAECONNRESET:           return io::Error::ConnectionReset;
    case WSAECONNABORTED:         return io::Error::ConnectionAborted;
    case WSAENOTCONN:             return io::Error::NotConnected;
    case WSAEADDRINUSE:           return io::Error::AddrInUse;
    case WSAEADDRNOTAVAIL:        return io::Error::AddrNotAvailable;
    case WSAENETUNREACH:          return io::Error::NetworkUnreachable;
    case WSAEHOSTUNREACH:         return io::Error::HostUnreachable;
    case WSAENETDOWN:             return io::Error::NetworkDown;
    case ERROR_IO_DEVICE:         return io::Error::BrokenPipe;
    case ERROR_FILE_EXISTS:
    case ERROR_ALREADY_EXISTS:    return io::Error::AlreadyExists;
    case WSAEWOULDBLOCK:          return io::Error::WouldBlock;
    case ERROR_INVALID_PARAMETER: return io::Error::InvalidInput;
    case ERROR_INVALID_FUNCTION:  return io::Error::InvalidOperation;
    case ERROR_OPERATION_ABORTED: return io::Error::Interrupted;
    case ERROR_NOT_SUPPORTED:     return io::Error::Unsupported;
    case ERROR_NO_MORE_FILES:
    case ERROR_HANDLE_EOF:        return io::Error::UnexpectedEof;
    case ERROR_DISK_FULL:         return io::Error::StorageFull;
    case ERROR_DIRECTORY:         return io::Error::NotADirectory;
    case ERROR_DIR_NOT_EMPTY:     return io::Error::DirectoryNotEmpty;
    case ERROR_DEVICE_IN_USE:     return io::Error::ResourceBusy;
    case ERROR_POSSIBLE_DEADLOCK: return io::Error::Deadlock;
    case ERROR_NOT_ENOUGH_MEMORY:
    case ERROR_OUTOFMEMORY:       return io::Error::StorageFull;
    case ERROR_IO_INCOMPLETE:     return io::Error::InProgress;
    case WSAETIMEDOUT:            return io::Error::TimedOut;
    default:                      return io::Error::Other;
  }
}

}  // namespace sfc::sys::windows
