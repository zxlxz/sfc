#include <Windows.h>

#include "sfc/ffi/wstr.h"
#include "sfc/sys/windows/io.h"

namespace sfc::sys::windows {

template <class R>
struct WBufReader {
  static constexpr usize BUF_LEN = 256U;
  R& _inn;
  wchar_t _buf[BUF_LEN] = {};
  ffi::WChars _wchars = {_buf, 0};

 public:
  auto read_char() -> io::Result<char32_t> {
    _TRY(this->fill_buf());

    const auto ch = _wchars.next();
    if (!ch) {
      return io::Error::UnexpectedEof;
    }
    return *ch;
  }

 private:
  auto fill_buf() -> io::Result<> {
    if (_wchars.len() > 0) {
      return Ok{};
    }

    const auto nread = _TRY(_inn.readw({_buf, BUF_LEN}));
    if (nread == 0) {
      return io::Error::UnexpectedEof;
    }

    _wchars = {_buf, nread};
    return Ok{};
  }
};

template <class W>
struct WBufWriter {
  static constexpr usize BUF_LEN = 256U;

  W& _inn;
  wchar_t _buf[BUF_LEN] = {};
  usize _len{0};

 public:
  auto write_char(char32_t ch) -> io::Result<> {
    if (_len + 2 >= BUF_LEN) {
      _TRY(flush());
    }
    const auto wstr = ffi::encode_wide({_buf + _len, BUF_LEN - _len}, ch);
    _len += wstr._len;
    return Ok{};
  }

  auto flush() -> io::Result<> {
    _TRY(_inn.writew({_buf, _len}));
    _len = 0;
    return Ok{};
  }
};

auto File::close() -> io::Result<> {
  if (_fd == nullptr || _fd == INVALID_HANDLE_VALUE) {
    return Ok{};
  }

  if (!::CloseHandle(_fd)) {
    return io::last_os_error();
  }

  _fd = nullptr;
  return Ok{};
}

auto File::read(Slice<u8> buf) -> io::Result<usize> {
  const auto buf_ptr = buf._ptr;
  const auto buf_len = num::saturating_cast<DWORD>(buf._len);

  auto nread = 0UL;
  if (!::ReadFile(_fd, buf_ptr, buf_len, &nread, nullptr)) {
    return io::last_os_error();
  }
  return usize{nread};
}

auto File::write(Slice<const u8> buf) -> io::Result<usize> {
  const auto buf_ptr = buf._ptr;
  const auto buf_len = num::saturating_cast<DWORD>(buf._len);

  auto nwrite = 0UL;
  if (!::WriteFile(_fd, buf_ptr, buf_len, &nwrite, nullptr)) {
    return io::last_os_error();
  }
  return usize{nwrite};
}

auto File::flush() -> io::Result<> {
  const auto ret = ::FlushFileBuffers(_fd);
  if (!ret) {
    return io::last_os_error();
  }
  return Ok{};
}

auto File::seek(io::SeekFrom pos) -> io::Result<u64> {
  const auto move_method = [=] -> DWORD {
    switch (pos._tag) {
      case io::SeekFrom::Kind::Start:   return FILE_BEGIN;
      case io::SeekFrom::Kind::Current: return FILE_CURRENT;
      case io::SeekFrom::Kind::End:     return FILE_END;
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

auto StdIo::Input() -> StdIo {
  const auto fd = ::GetStdHandle(STD_INPUT_HANDLE);
  return StdIo{{fd}};
}

auto StdIo::Output() -> StdIo {
  const auto fd = ::GetStdHandle(STD_OUTPUT_HANDLE);
  return StdIo{{fd}};
}

auto StdIo::Error() -> StdIo {
  const auto fd = ::GetStdHandle(STD_ERROR_HANDLE);
  return StdIo{{fd}};
}

auto StdIo::is_console() const -> bool {
  auto mode = DWORD{};
  return ::GetConsoleMode(_fd, &mode);
}

auto StdIo::is_utf8_console() const -> bool {
  const auto code_page = ::GetConsoleCP();
  return code_page == CP_UTF8;
}

auto StdIo::flush() -> io::Result<> {
  return Ok{};
}

auto StdIo::readw(Slice<wchar_t> buf) -> io::Result<usize> {
  const auto buf_ptr = buf._ptr;
  const auto buf_len = num::saturating_cast<DWORD>(buf._len);

  auto nret = DWORD{};
  if (!::ReadConsoleW(_fd, buf_ptr, buf_len, &nret, nullptr)) {
    return io::last_os_error();
  }
  return usize{nret};
}

auto StdIo::writew(Slice<const wchar_t> buf) -> io::Result<usize> {
  const auto buf_ptr = buf._ptr;
  const auto buf_len = num::saturating_cast<DWORD>(buf._len);

  auto nret = DWORD{};
  if (!::WriteConsoleW(_fd, buf_ptr, buf_len, &nret, nullptr)) {
    return Err{io::last_os_error()};
  }

  return usize{buf._len};
}

auto StdIo::read(Slice<u8> buf) -> io::Result<usize> {
  if (this->is_utf8_console()) {
    return File::read(buf);
  }

  auto rbuf = WBufReader{*this};
  auto tmp = buf;
  while (tmp.len() >= 3) {
    const auto ch = _TRY(rbuf.read_char());
    const auto cnt = chr::encode_utf8(tmp, ch)._len;
    tmp = tmp[{cnt, $}];
  }

  return usize{buf._len - tmp._len};
}

auto StdIo::write(Slice<const u8> buf) -> io::Result<usize> {
  if (this->is_utf8_console()) {
    return File::write(buf);
  }

  auto wbuf = WBufWriter{*this};
  auto chars = Str::from_utf8(buf).chars();
  while (auto ch = chars.next()) {
    _TRY(wbuf.write_char(*ch));
  }
  _TRY(wbuf.flush());
  return usize{buf._len};
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
