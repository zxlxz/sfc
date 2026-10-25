#include <Windows.h>

#include "sfc/ffi/wstr.h"
#include "sfc/sys/windows/io.h"

namespace sfc::sys::windows {

using chr::U16Chars;

template <class R>
struct WBufReader {
  static constexpr auto kBufLen = 256U;
  R& _inn;
  u16 _buf[kBufLen] = {};
  U16Chars _rem = {};

 public:
  auto read(Slice<u8> buf) -> io::Result<usize> {
    auto rem = buf;
    while (true) {
      const auto rem_utf8_cap = rem._len;
      const auto rem_chars_cap = (rem_utf8_cap + 2) / 3;
      if (rem_chars_cap == 0) {
        break;
      }

      _TRY(this->fill_buf(rem_chars_cap));

      const auto ch = _rem.next();
      if (!ch) {
        break;
      }

      const auto v = chr::encode_utf8(rem, *ch);
      if (v._len == 0) {
        break;
      }
      rem = Slice{rem._ptr + v._len, rem._len - v._len};
    }

    return Ok{buf._len - rem._len};
  }

 private:
  auto fill_buf(usize max_read = kBufLen) -> io::Result<usize> {
    if (_rem._buf.is_empty()) {
      max_read = cmp::min(max_read, kBufLen);
      const auto nread = _TRY(_inn.read_u16({_buf, max_read}));
      _rem = U16Chars{{}, {_buf, nread}};
    }

    return Ok{_rem._buf._len};
  }
};

template <class W>
struct WBufWriter {
  static constexpr auto kBufLen = 256U;
  W& _inn;
  u16 _buf[kBufLen] = {};
  usize _len = 0;

 public:
  auto write(Slice<const u8> buf) -> io::Result<usize> {
    // convert: utf8 -> chars
    auto chars = chr::decode_utf8(buf);

    // convert: chars -> utf16
    while (auto ch = chars.next()) {
      _TRY(this->write_char(*ch));
    }

    _TRY(this->flush());

    const auto nwrite = buf.len() - chars._buf.len();
    return Ok{nwrite};
  }

  auto flush() -> io::Result<> {
    if (_len == 0) {
      return Ok{};
    }

    auto rem = Slice{_buf, _len};
    while (!rem.is_empty()) {
      const auto nwrite = _TRY(_inn.write_u16(rem));
      if (nwrite == 0) {
        return io::Error::UnexpectedEof;
      }
      rem = Slice{rem._ptr + nwrite, rem._len - nwrite};
    }

    _len = 0;
    return Ok{};
  }

 private:
  auto write_char(char32_t ch) -> io::Result<> {
    if (_len + 2 > kBufLen) {
      _TRY(this->flush());
    }

    const auto v = chr::encode_utf16({_buf + _len, 2}, ch);
    _len += v._len;
    return Ok{};
  }
};

void File::close() {
  if (_fd == nullptr || _fd == INVALID_HANDLE_VALUE) {
    return;
  }
  ::CloseHandle(_fd);
  _fd = nullptr;
}

auto File::read(Slice<u8> buf) -> io::Result<usize> {
  const auto buf_ptr = buf._ptr;
  const auto buf_len = u32(cmp::min(buf._len, U32::MAX));

  auto nread = 0UL;
  if (!::ReadFile(_fd, buf_ptr, buf_len, &nread, nullptr)) {
    return io::last_os_error();
  }
  return usize{nread};
}

auto File::write(Slice<const u8> buf) -> io::Result<usize> {
  const auto buf_ptr = buf._ptr;
  const auto buf_len = u32(cmp::min(buf._len, U32::MAX));

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
  const auto handle = ::GetStdHandle(STD_INPUT_HANDLE);
  return StdIo{._file = {handle}};
}

auto StdIo::Output() -> StdIo {
  const auto handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
  return StdIo{._file = {handle}};
}

auto StdIo::Error() -> StdIo {
  const auto handle = ::GetStdHandle(STD_ERROR_HANDLE);
  return StdIo{._file = {handle}};
}

auto StdIo::is_console() const -> bool {
  auto mode = DWORD{};
  return ::GetConsoleMode(_file._fd, &mode);
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

  // limit max read len of characters to U32::MAX to meet Windows API requirements
  const auto max_read = u32(cmp::min(buf._len, U32::MAX));

  auto nread = DWORD{};
  if (!::ReadConsoleW(_file._fd, buf_ptr, max_read, &nread, nullptr)) {
    return io::last_os_error();
  }
  return usize{nread};
}

auto StdIo::write_u16(Slice<const u16> buf) -> io::Result<usize> {
  const auto buf_ptr = buf._ptr;

  // limit max write len of characters to U32::MAX to meet Windows API requirements
  const auto max_write = u32(cmp::min(buf._len, U32::MAX));

  auto nwrite = DWORD{};
  if (!::WriteConsoleW(_file._fd, buf_ptr, max_write, &nwrite, nullptr)) {
    return io::last_os_error();
  }

  return usize{nwrite};
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
