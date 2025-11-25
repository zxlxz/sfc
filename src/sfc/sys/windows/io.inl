#pragma once
#ifdef _WIN32
#include <Windows.h>

namespace sfc::sys::io {

static const auto INVALID_FD = INVALID_HANDLE_VALUE;

static inline auto get_err() -> int {
  return static_cast<int>(::GetLastError());
}

template <class T>
static inline auto kind_of(int code) -> T {
  switch (code) {
    case ERROR_ACCESS_DENIED:              return T::PermissionDenied;
    case ERROR_ALREADY_EXISTS:             return T::AlreadyExists;
    case ERROR_FILE_EXISTS:                return T::AlreadyExists;
    case ERROR_FILE_NOT_FOUND:             return T::NotFound;
    case ERROR_PATH_NOT_FOUND:             return T::NotFound;
    case ERROR_INVALID_HANDLE:             return T::InvalidInput;
    case ERROR_NOT_ENOUGH_MEMORY:          return T::BrokenPipe;
    case ERROR_BROKEN_PIPE:                return T::BrokenPipe;
    case ERROR_DISK_FULL:                  return T::Other;
    case ERROR_IO_PENDING:                 return T::WouldBlock;
    case ERROR_OPERATION_ABORTED:          return T::Interrupted;
    case ERROR_TIMEOUT:                    return T::TimedOut;
    case ERROR_INVALID_DATA:               return T::InvalidData;
    case ERROR_INVALID_PARAMETER:          return T::InvalidInput;
    case ERROR_NOT_CONNECTED:              return T::NotConnected;
    case ERROR_CONNECTION_REFUSED:         return T::ConnectionRefused;
    case ERROR_CONNECTION_ABORTED:         return T::ConnectionAborted;
    case ERROR_ADDRESS_ALREADY_ASSOCIATED: return T::AddrInUse;
    case ERROR_ADDRESS_NOT_ASSOCIATED:     return T::AddrNotAvailable;
    case ERROR_NO_DATA:                    return T::UnexpectedEof;
    case ERROR_NOACCESS:                   return T::InvalidInput;
    default:                               return T::Other;
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
