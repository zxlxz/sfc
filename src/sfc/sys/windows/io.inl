#pragma once

#include <Windows.h>

#include "sfc/io/mod.h"

namespace sfc::sys::io {

struct File {
  void* _fd = INVALID_HANDLE_VALUE;

 public:
  operator bool() const {
    return _fd != INVALID_HANDLE_VALUE;
  }

  void close() {
    if (_fd == INVALID_HANDLE_VALUE) {
      return;
    }
    ::CloseHandle(_fd);
  }

  auto read(Slice<u8> buf) -> LONG64 {
    auto bytes_read = 0UL;
    if (::ReadFile(_fd, buf.as_mut_ptr(), buf.len(), &bytes_read, nullptr) == 0) {
      return -1;
    }
    return static_cast<LONG64>(bytes_read);
  }

  auto write(Slice<const u8> buf) -> LONG64 {
    auto bytes_write = 0UL;
    if (::WriteFile(_fd, buf.as_ptr(), buf.len(), &bytes_write, nullptr) == 0) {
      return -1;
    }
    return static_cast<LONG64>(bytes_write);
  }
};

struct Stdout : File {
  explicit Stdout() : File{::GetStdHandle(STD_OUTPUT_HANDLE)} {}

  ~Stdout() {}

  auto is_tty() -> bool {
    auto mode = 0UL;
    return ::GetConsoleMode(_fd, &mode) != 0;
  }

  void flush() {
    return;
  }
};

static inline auto last_err() -> int {
  return static_cast<int>(GetLastError());
}

static inline auto error_str(int err_code) -> sfc::str::Str {
  static thread_local char buf[1024];

  const auto num_chars =
      ::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       nullptr,
                       err_code,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       buf,
                       sizeof(buf),
                       nullptr);
  if (num_chars == 0) {
    return {};
  }

  return {buf, num_chars};
}

static inline auto err_kind(int code) -> sfc::io::ErrorKind {
  // clang-format off
  switch (code) {
    case ERROR_ACCESS_DENIED:     return sfc::io::ErrorKind::PermissionDenied;
    case ERROR_ALREADY_EXISTS:    return sfc::io::ErrorKind::AlreadyExists;
    case ERROR_FILE_EXISTS:       return sfc::io::ErrorKind::AlreadyExists;
    case ERROR_FILE_NOT_FOUND:    return sfc::io::ErrorKind::NotFound;
    case ERROR_PATH_NOT_FOUND:    return sfc::io::ErrorKind::NotFound;
    case ERROR_INVALID_HANDLE:    return sfc::io::ErrorKind::InvalidInput;
    case ERROR_NOT_ENOUGH_MEMORY: return sfc::io::ErrorKind::BrokenPipe;
    case ERROR_BROKEN_PIPE:       return sfc::io::ErrorKind::BrokenPipe;
    case ERROR_DISK_FULL:         return sfc::io::ErrorKind::Other;
    case ERROR_IO_PENDING:        return sfc::io::ErrorKind::WouldBlock;
    case ERROR_OPERATION_ABORTED: return sfc::io::ErrorKind::Interrupted;
    case ERROR_TIMEOUT:           return sfc::io::ErrorKind::TimedOut;
    case ERROR_INVALID_DATA:      return sfc::io::ErrorKind::InvalidData;
    default:                      return sfc::io::ErrorKind::Other;
  }
  // clang-format on
}

}  // namespace sfc::sys::io
