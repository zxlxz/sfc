#include <Windows.h>
#undef min
#undef max

#include "sfc/ffi/wstr.h"
#include "sfc/sys/windows/io.h"

namespace sfc::sys::windows {

File::File(void* fd) noexcept : _fd(fd) {}

File::~File() noexcept {
  if (_fd == nullptr || _fd == INVALID_HANDLE_VALUE) return;
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

auto File::is_valid() const noexcept -> bool {
  return _fd != nullptr && _fd != INVALID_HANDLE_VALUE;
}

auto File::flush() noexcept -> io::Result<> {
  const auto ret = ::FlushFileBuffers(_fd);
  if (!ret) {
    return {io::last_os_error()};
  }
  return Ok{};
}

auto File::read(Slice<u8> buf) noexcept -> io::Result<usize> {
  const auto buf_ptr = buf._ptr;
  const auto buf_len = num::saturating_cast<DWORD>(buf._len);

  auto nread = 0UL;
  if (!::ReadFile(_fd, buf_ptr, buf_len, &nread, nullptr)) {
    return io::last_os_error();
  }
  return usize{nread};
}

auto File::write(Slice<const u8> buf) noexcept -> io::Result<usize> {
  const auto buf_ptr = buf._ptr;
  const auto buf_len = num::saturating_cast<DWORD>(buf._len);

  auto nwrite = 0UL;
  if (!::WriteFile(_fd, buf_ptr, buf_len, &nwrite, nullptr)) {
    return io::last_os_error();
  }
  return usize{nwrite};
}

auto File::seek(io::SeekFrom pos) noexcept -> io::Result<u64> {
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

struct StdIo {
  static constexpr auto kMaxBufLen = 4096UL;
  HANDLE _handle;

 public:
  static auto Input() -> StdIo& {
    static auto res = StdIo{::GetStdHandle(STD_INPUT_HANDLE)};
    return res;
  }

  static auto Output() -> StdIo& {
    static auto res = StdIo{::GetStdHandle(STD_OUTPUT_HANDLE)};
    return res;
  }

  static auto Error() -> StdIo& {
    static auto res = StdIo{::GetStdHandle(STD_ERROR_HANDLE)};
    return res;
  }

 public:
  auto is_console() const -> bool {
    auto mode = DWORD{};
    return ::GetConsoleMode(_handle, &mode);
  }

  auto is_utf8_console() const -> bool {
    const auto code_page = ::GetConsoleCP();
    return code_page == CP_UTF8;
  }

  auto write_u8(Str u8_str) -> io::Result<usize> {
    const auto buf = u8_str._ptr;
    const auto buf_len = num::saturating_cast<DWORD>(u8_str._len);

    auto nret = DWORD{};
    if (!::WriteFile(_handle, buf, buf_len, &nret, nullptr)) {
      return {io::last_os_error()};
    }
    return {usize{nret}};
  }

  auto write_u16(Str u8_str) -> io::Result<usize> {
    wchar_t wbuf[kMaxBufLen];
    u32 wlen = 0U;
    u8_str.chars().for_each([&](char32_t ch) {
      if (wlen >= kMaxBufLen) return;
      const auto n = ffi::wide_encode(ch, {wbuf + wlen, kMaxBufLen - wlen});
      wlen += u32(n);
    });

    auto nwrite = DWORD{};
    if (!::WriteConsoleW(_handle, wbuf, wlen, &nwrite, nullptr)) {
      return {io::last_os_error()};
    }
    return {usize{nwrite}};
  }

  auto write(Slice<const u8> data) -> io::Result<usize> {
    const auto u8_str = Str::from_utf8(data);
    if (!this->is_console() || this->is_utf8_console()) {
      return this->write_u8(u8_str);
    } else {
      return this->write_u16(u8_str);
    }
  }

  auto read_u8(Slice<u8> data) -> io::Result<usize> {
    const auto buf = data._ptr;
    const auto buf_len = num::saturating_cast<DWORD>(data._len);

    auto nret = DWORD{};
    if (!::ReadFile(_handle, buf, buf_len, &nret, nullptr)) {
      return io::last_os_error();
    }
    return usize{nret};
  }

  auto read_u16(Slice<u8> data) -> io::Result<usize> {
    // in the worst case, each u16 character takes 3 u8 bytes
    const auto max_wchar = num::saturating_cast<DWORD>(data._len / 3);
    const auto max_read = cmp::min(max_wchar, kMaxBufLen);

    // read u16
    wchar_t wbuf[kMaxBufLen];
    auto nret = DWORD{};
    if (!::ReadConsoleW(_handle, wbuf, max_read, &nret, nullptr)) {
      return io::last_os_error();
    }

    const auto wstr = ffi::WStr{wbuf, nret};

    // convert u16 to u8
    auto u8_ptr = data._ptr;
    auto u8_cap = data._len;
    auto u8_len = 0UL;
    wstr.chars().for_each([&](char32_t ch) {
      const auto n = chr::utf8_encode(ch, {u8_ptr + u8_len, u8_cap - u8_len});
      u8_len += n;
    });

    return io::Result<usize>{u8_len};
  }

  auto read(Slice<u8> data) -> io::Result<usize> {
    if (!this->is_console() || this->is_utf8_console()) {
      return this->read_u8(data);
    } else {
      return this->read_u16(data);
    }
  }
};

auto StdIn::is_console() -> bool {
  auto& imp = StdIo::Input();
  return imp.is_console();
}

auto StdIn::read(Slice<u8> data) -> io::Result<usize> {
  auto& imp = StdIo::Input();
  return imp.read(data);
}

auto Stdout::is_console() -> bool {
  auto& imp = StdIo::Output();
  return imp.is_console();
}

auto Stdout::write(Slice<const u8> buf) -> io::Result<usize> {
  auto& imp = StdIo::Output();
  return imp.write(buf);
}

auto Stderr::is_console() -> bool {
  auto& imp = StdIo::Error();
  return imp.is_console();
}

auto Stderr::write(Slice<const u8> buf) -> io::Result<usize> {
  auto& imp = StdIo::Error();
  return imp.write(buf);
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
