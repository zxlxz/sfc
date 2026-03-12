#pragma once
#include "sfc/sys/windows/mod.inl"

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

struct File {
  HANDLE _fd;

 public:
  auto is_valid() const -> bool {
    return _fd != nullptr && _fd != INVALID_HANDLE_VALUE;
  }

  auto is_tty() const -> bool {
    if (_fd == nullptr || _fd == INVALID_HANDLE_VALUE) {
      return false;
    }

    auto mode = 0UL;
    if (!::GetConsoleMode(_fd, &mode)) {
      return false;
    }
    return mode != 0;
  }

  void close() {
    if (_fd == nullptr || _fd == INVALID_HANDLE_VALUE) {
      return;
    }
    (void)::CloseHandle(_fd);
    _fd = nullptr;
  }

  auto flush() -> io::Result<> {
    const auto ret = ::FlushFileBuffers(_fd);
    if (!ret) {
      return io::last_os_error();
    }
    return {};
  }

  auto read(Slice<u8> buf) -> io::Result<usize> {
    auto bytes_read = 0UL;
    if (!::ReadFile(_fd, buf._ptr, static_cast<DWORD>(buf._len), &bytes_read, nullptr)) {
      return io::last_os_error();
    }
    return static_cast<usize>(bytes_read);
  }

  auto write(Slice<const u8> buf) -> io::Result<usize> {
    auto bytes_written = 0UL;
    if (!::WriteFile(_fd, buf._ptr, static_cast<DWORD>(buf._len), &bytes_written, nullptr)) {
      return io::last_os_error();
    }
    return static_cast<usize>(bytes_written);
  }

  auto seek(SSIZE_T offset, DWORD whence) -> io::Result<usize> {
    const auto old_offset = LARGE_INTEGER{.QuadPart = offset};
    auto new_offset = LARGE_INTEGER{};
    if (!::SetFilePointerEx(_fd, old_offset, &new_offset, whence)) {
      return io::last_os_error();
    }
    return static_cast<usize>(new_offset.QuadPart);
  }
};

static inline auto stdout() -> File {
  static const auto handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
  return File{handle};
}

static inline auto stderr() -> File {
  static const auto handle = ::GetStdHandle(STD_ERROR_HANDLE);
  return File{handle};
}

static inline auto stdin() -> File {
  static const auto handle = ::GetStdHandle(STD_INPUT_HANDLE);
  return File{handle};
}

}  // namespace sfc::sys::windows
