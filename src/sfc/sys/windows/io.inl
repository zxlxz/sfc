#pragma once
#ifdef _WIN32
#include <Windows.h>
#include <winerror.h>

namespace sfc::sys::io {

static const auto INVALID_FD = INVALID_HANDLE_VALUE;

static inline auto get_err() -> int {
  return static_cast<int>(::GetLastError());
}

template <class T>
static inline auto map_err(int code) -> T {
  switch (code) {
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:    return T::NotFound;
    case ERROR_ACCESS_DENIED:     return T::PermissionDenied;
    case WSAECONNREFUSED:         return T::ConnectionRefused;
    case WSAECONNRESET:           return T::ConnectionReset;
    case WSAECONNABORTED:         return T::ConnectionAborted;
    case WSAENOTCONN:             return T::NotConnected;
    case WSAEADDRINUSE:           return T::AddrInUse;
    case WSAEADDRNOTAVAIL:        return T::AddrNotAvailable;
    case WSAENETUNREACH:          return T::NetworkUnreachable;
    case WSAEHOSTUNREACH:         return T::HostUnreachable;
    case WSAENETDOWN:             return T::NetworkDown;
    case ERROR_IO_DEVICE:         return T::BrokenPipe;
    case ERROR_FILE_EXISTS:
    case ERROR_ALREADY_EXISTS:    return T::AlreadyExists;
    case WSAEWOULDBLOCK:          return T::WouldBlock;
    case ERROR_INVALID_PARAMETER: return T::InvalidInput;
    case ERROR_INVALID_FUNCTION:  return T::InvalidOperation;
    case ERROR_OPERATION_ABORTED: return T::Interrupted;
    case ERROR_NOT_SUPPORTED:     return T::Unsupported;
    case ERROR_HANDLE_EOF:        return T::UnexpectedEof;
    case ERROR_DISK_FULL:         return T::StorageFull;
    case ERROR_DIRECTORY:         return T::NotADirectory;
    case ERROR_DIR_NOT_EMPTY:     return T::DirectoryNotEmpty;
    case ERROR_DEVICE_IN_USE:     return T::ResourceBusy;
    case ERROR_POSSIBLE_DEADLOCK: return T::Deadlock;
    case ERROR_NOT_ENOUGH_MEMORY:
    case ERROR_OUTOFMEMORY:       return T::StorageFull;
    case ERROR_IO_INCOMPLETE:     return T::InProgress;
    case WSAETIMEDOUT:            return T::TimedOut;
    default:                      return T::Other;
  }
}

static inline void close(HANDLE fd) {
  if (fd == nullptr || fd == INVALID_HANDLE_VALUE) {
    return;
  }
  ::CloseHandle(fd);
}

static inline void flush(HANDLE fd) {
  if (fd == nullptr || fd == INVALID_HANDLE_VALUE) {
    return;
  }
  ::FlushFileBuffers(fd);
}

static inline auto read(HANDLE fd, void* buf, SIZE_T buf_size) -> SSIZE_T {
  if (fd == nullptr || fd == INVALID_HANDLE_VALUE) {
    return -1;
  }

  if (buf == nullptr || buf_size == 0) {
    return 0;
  }

  auto bytes_read = 0UL;
  if (!::ReadFile(fd, buf, static_cast<DWORD>(buf_size), &bytes_read, nullptr)) {
    return -1;
  }

  return bytes_read;
};

static inline auto write(HANDLE fd, const void* buf, SIZE_T buf_size) -> SSIZE_T {
  if (fd == nullptr || fd == INVALID_HANDLE_VALUE) {
    return -1;
  }

  if (buf == nullptr || buf_size == 0) {
    return 0;
  }

  auto bytes_write = 0UL;
  if (!::WriteFile(fd, buf, static_cast<DWORD>(buf_size), &bytes_write, nullptr)) {
    return -1;
  }
  return bytes_write;
}

static inline auto is_tty(HANDLE fd) -> bool {
  if (fd == INVALID_HANDLE_VALUE) {
    return false;
  }

  auto mode = 0UL;
  if (!::GetConsoleMode(fd, &mode)) {
    return false;
  }
  return mode != 0;
}

static inline auto stdout() -> HANDLE {
  return ::GetStdHandle(STD_OUTPUT_HANDLE);
}

static inline auto stderr() -> HANDLE {
  return ::GetStdHandle(STD_ERROR_HANDLE);
}

static inline auto stdin() -> HANDLE {
  return ::GetStdHandle(STD_INPUT_HANDLE);
}

}  // namespace sfc::sys::io
#endif
