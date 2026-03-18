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

struct StdIo {
  static constexpr auto MAX_BUF_LEN = 4096U;
  HANDLE _handle;

 public:
  auto is_console() const -> bool {
    auto mode = DWORD{};
    return ::GetConsoleMode(_handle, &mode);
  }

  auto is_utf8_console() const -> bool {
    return ::GetConsoleCP() == CP_UTF8;
  }

  auto write_u8(Slice<const u8> data) -> io::Result<usize> {
    auto nwrite = DWORD{};
    if (!::WriteFile(_handle, data._ptr, static_cast<DWORD>(data._len), &nwrite, nullptr)) {
      return io::last_os_error();
    }
    return nwrite;
  }

  auto write_u16(Slice<const u8> data) -> io::Result<usize> {
    const auto u8_ptr = reinterpret_cast<const char*>(data._ptr);
    const auto u8_len = static_cast<DWORD>(data._len);

    // convert u8 to u16
    wchar_t u16_buf[MAX_BUF_LEN];
    const auto u16_len = ::MultiByteToWideChar(CP_UTF8, 0, u8_ptr, u8_len, u16_buf, MAX_BUF_LEN);
    if (u16_len == 0) {
      return io::last_os_error();
    }

    auto nwrite = DWORD{};
    if (!::WriteConsoleW(_handle, u16_buf, u16_len, &nwrite, nullptr)) {
      return io::last_os_error();
    }
    return static_cast<usize>(nwrite);
  }

  auto write(Slice<const u8> data) -> io::Result<usize> {
    if (!this->is_console() || this->is_utf8_console()) {
      return this->write_u8(data);
    } else {
      return this->write_u16(data);
    }
  }

  auto read_u8(Slice<u8> data) -> io::Result<usize> {
    auto nread = DWORD{};
    if (!::ReadFile(_handle, data._ptr, static_cast<DWORD>(data._len), &nread, nullptr)) {
      return io::last_os_error();
    }
    return nread;
  }

  auto read_u16(Slice<u8> data) -> io::Result<usize> {
    // in the worst case, each u16 character takes 3 u8 bytes
    const auto amount = cmp::min(static_cast<u32>(data.len() / 3), MAX_BUF_LEN);

    // read u16
    wchar_t u16_buf[MAX_BUF_LEN];
    auto u16_len = DWORD{};
    if (!::ReadConsoleW(_handle, u16_buf, amount, &u16_len, nullptr)) {
      return io::last_os_error();
    }

    // convert u16 to u8
    const auto u8_ptr = reinterpret_cast<char*>(data._ptr);
    const auto u8_cap = static_cast<DWORD>(data._len);
    const auto u8_len = ::WideCharToMultiByte(CP_UTF8, 0, u16_buf, u16_len, u8_ptr, u8_cap, nullptr, nullptr);
    if (u8_len == 0) {
      return io::last_os_error();
    }

    return static_cast<usize>(u8_len);
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
    return {};
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
    return {};
  }
};

}  // namespace sfc::sys::windows
