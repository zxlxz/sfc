#pragma once
#include "sfc/sys/windows/mod.inl"
#define _SFC_SYS_IO_

namespace sfc::sys::windows {

static inline auto io_error(DWORD code) -> io::Error {
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

static inline auto wstr_encode(Slice<wchar_t> wbuf, Str cstr) -> DWORD {
  auto wlen = 0UL;
  cstr.chars().for_each([&](char32_t ch) {
    wchar_t buf[2];
    const auto cnt = ffi::wide_encode(buf, ch);
    if (wlen + cnt >= wbuf._len) return;
    ptr::copy_nonoverlapping(buf, wbuf._ptr + wlen, cnt);
  });

  return wlen;
}

static inline auto wstr_decode(Slice<u8> cbuf, ffi::WStr wstr) -> usize {
  auto u8_len = 0UL;

  wstr.chars().for_each([&](char32_t ch) {
    u8 buf[4];
    const auto cnt = chr::utf8_encode(buf, ch);
    if (u8_len + cnt >= cbuf._len) return;
    ptr::copy_nonoverlapping(buf, cbuf._ptr + u8_len, cnt);
  });

  return u8_len;
}

struct StdIo {
  static constexpr auto kMaxBufLen = 4096UL;
  HANDLE _handle;

 public:
  auto is_console() const -> bool {
    auto mode = DWORD{};
    return ::GetConsoleMode(_handle, &mode);
  }

  auto is_utf8_console() const -> bool {
    return ::GetConsoleCP() == CP_UTF8;
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
    wchar_t buf[kMaxBufLen];
    const auto wlen = windows::wstr_encode(buf, u8_str);

    auto nwrite = DWORD{};
    if (!::WriteConsoleW(_handle, buf, wlen, &nwrite, nullptr)) {
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
      return io::Result<usize>{io::last_os_error()};
    }
    return io::Result<usize>{usize{nret}};
  }

  auto read_u16(Slice<u8> data) -> io::Result<usize> {
    // in the worst case, each u16 character takes 3 u8 bytes
    const auto max_wchar = num::saturating_cast<DWORD>(data._len / 3);
    const auto max_read = cmp::min(max_wchar, kMaxBufLen);

    // read u16
    wchar_t buf[kMaxBufLen];
    auto nret = DWORD{};
    if (!::ReadConsoleW(_handle, buf, max_read, &nret, nullptr)) {
      return io::Result<usize>{io::last_os_error()};
    }

    // convert u16 to u8
    const auto ws_len = usize{nret};
    const auto u8_len = windows::wstr_decode(data, {buf, ws_len});
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

struct StdIn {
  static auto read(Slice<u8> data) -> io::Result<usize> {
    const auto handle = ::GetStdHandle(STD_INPUT_HANDLE);
    return StdIo{handle}.read(data);
  }
};

struct Stdout {
  static auto is_console() -> bool {
    const auto handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
    return StdIo{handle}.is_console();
  }

  static auto write(Slice<const u8> data) -> io::Result<usize> {
    const auto handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
    return StdIo{handle}.write(data);
  }

  static auto flush() -> io::Result<> {
    return Tuple{};
  }
};

struct Stderr {
  static auto is_console() -> bool {
    const auto handle = ::GetStdHandle(STD_ERROR_HANDLE);
    return StdIo{handle}.is_console();
  }

  static auto write(Slice<const u8> data) -> io::Result<usize> {
    const auto handle = ::GetStdHandle(STD_ERROR_HANDLE);
    return StdIo{handle}.write(data);
  }

  static auto flush() -> io::Result<> {
    return Tuple{};
  }
};

}  // namespace sfc::sys::windows
