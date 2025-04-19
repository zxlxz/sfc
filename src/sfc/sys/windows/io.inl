#pragma once

#include <Windows.h>

#include "sfc/io/mod.h"

namespace sfc::sys::io {

using error_t = DWORD;

struct Error {
  DWORD _code = 0;

 public:
  static auto last() -> Error {
    return {::GetLastError()};
  }

  auto code() const -> int {
    return static_cast<int>(_code);
  }

  auto kind() const -> sfc::io::ErrorKind {
    switch (_code) {
        // clang-format off
    case ERROR_ACCESS_DENIED:               return sfc::io::ErrorKind::PermissionDenied;
    case ERROR_ALREADY_EXISTS:              return sfc::io::ErrorKind::AlreadyExists;
    case ERROR_FILE_EXISTS:                 return sfc::io::ErrorKind::AlreadyExists;
    case ERROR_FILE_NOT_FOUND:              return sfc::io::ErrorKind::NotFound;
    case ERROR_PATH_NOT_FOUND:              return sfc::io::ErrorKind::NotFound;
    case ERROR_INVALID_HANDLE:              return sfc::io::ErrorKind::InvalidInput;
    case ERROR_NOT_ENOUGH_MEMORY:           return sfc::io::ErrorKind::BrokenPipe;
    case ERROR_BROKEN_PIPE:                 return sfc::io::ErrorKind::BrokenPipe;
    case ERROR_DISK_FULL:                   return sfc::io::ErrorKind::Other;
    case ERROR_IO_PENDING:                  return sfc::io::ErrorKind::WouldBlock;
    case ERROR_OPERATION_ABORTED:           return sfc::io::ErrorKind::Interrupted;
    case ERROR_TIMEOUT:                     return sfc::io::ErrorKind::TimedOut;
    case ERROR_INVALID_DATA:                return sfc::io::ErrorKind::InvalidData;
    case ERROR_INVALID_PARAMETER:           return sfc::io::ErrorKind::InvalidInput;
    case ERROR_NOT_CONNECTED:               return sfc::io::ErrorKind::NotConnected;
    case ERROR_CONNECTION_REFUSED:          return sfc::io::ErrorKind::ConnectionRefused;
    case ERROR_CONNECTION_ABORTED:          return sfc::io::ErrorKind::ConnectionAborted;
    case ERROR_ADDRESS_ALREADY_ASSOCIATED:  return sfc::io::ErrorKind::AddrInUse;
    case ERROR_ADDRESS_NOT_ASSOCIATED:      return sfc::io::ErrorKind::AddrNotAvailable;
    case ERROR_NO_DATA:                     return sfc::io::ErrorKind::UnexpectedEof;
    case ERROR_NOACCESS:                    return sfc::io::ErrorKind::InvalidInput;
   default:                                 return sfc::io::ErrorKind::Other;
        // clang-format on
    }
  }
};

struct File {
  void* _fd = INVALID_HANDLE_VALUE;

 public:
  explicit operator bool() const {
    return _fd != INVALID_HANDLE_VALUE;
  }

  void close() {
    if (_fd == INVALID_HANDLE_VALUE) {
      return;
    }
    ::CloseHandle(_fd);
  }

  void flush() {
    if (_fd == INVALID_HANDLE_VALUE) {
      return;
    }
    ::FlushFileBuffers(_fd);
  }

  auto read(void* buf, SIZE_T buf_size) -> LONG64 {
    if (buf == nullptr || buf_size == 0) {
      return 0;
    }

    if (_fd == INVALID_HANDLE_VALUE) {
      return -1;
    }

    auto bytes_read = 0UL;
    if (::ReadFile(_fd, buf, static_cast<DWORD>(buf_size), &bytes_read, nullptr) == 0) {
      return -1;
    }

    return static_cast<LONG64>(bytes_read);
  }

  auto write(const void* buf, SIZE_T buf_size) -> LONG64 {
    if (buf == nullptr || buf_size == 0) {
      return 0;
    }

    if (_fd == INVALID_HANDLE_VALUE) {
      return -1;
    }

    auto bytes_write = 0UL;
    if (::WriteFile(_fd, buf, buf_size, &bytes_write, nullptr) == 0) {
      return -1;
    }
    return static_cast<LONG64>(bytes_write);
  }

  auto is_tty() const -> bool {
    if (_fd == INVALID_HANDLE_VALUE) {
      return false;
    }

    auto mode = 0UL;
    return ::GetConsoleMode(_fd, &mode) == TRUE;
  }
};

struct OpenOptions {
  bool append;
  bool create;
  bool create_new;
  bool read;
  bool write;
  bool truncate;

 public:
  auto access_mode() const -> DWORD {
    return append ? FILE_APPEND_DATA : write ? GENERIC_WRITE : GENERIC_READ;
  }

  auto share_mode() const -> DWORD {
    return FILE_SHARE_READ;
  }

  auto create_mode() const -> DWORD {
    return create_new ? CREATE_NEW : write ? CREATE_ALWAYS : OPEN_EXISTING;
  }

  auto file_attrs() const -> DWORD {
    return FILE_ATTRIBUTE_NORMAL;
  }

  auto file_flags() const -> DWORD {
    return FILE_FLAG_OVERLAPPED;
  }

  auto open(const char* path) const -> HANDLE {
    const auto handle = ::CreateFileA(path,
                                      access_mode(),
                                      share_mode(),
                                      nullptr,
                                      create_mode(),
                                      file_attrs(),
                                      nullptr);
    return handle;
  }
};

struct FileAttr {
  DWORD  _attr = 0;
  SIZE_T _size = 0;

 public:
  operator bool() const {
    return _attr != 0;
  }

  auto size() const -> SIZE_T {
    return _size;
  }

  auto is_dir() const -> bool {
    return _attr & FILE_ATTRIBUTE_DIRECTORY;
  }

  auto is_file() const -> bool {
    return _attr & FILE_ATTRIBUTE_NORMAL;
  }
};

static inline auto stdout() -> File& {
  static auto res = File{::GetStdHandle(STD_OUTPUT_HANDLE)};
  return res;
}

static inline auto stderr() -> File& {
  static auto res = File{::GetStdHandle(STD_ERROR_HANDLE)};
  return res;
}

static inline auto stdin() -> File& {
  static auto res = File{::GetStdHandle(STD_INPUT_HANDLE)};
  return res;
}

static inline auto lstat(const char* path) -> FileAttr {
  auto attr = WIN32_FILE_ATTRIBUTE_DATA{};
  if (::GetFileAttributesExA(path, GetFileExInfoStandard, &attr) == 0) {
    return {};
  }

  const auto file_size = SIZE_T(attr.nFileSizeHigh) + (SIZE_T(attr.nFileSizeLow) << 32);
  return FileAttr{attr.dwFileAttributes, file_size};
}

static inline auto unlink(const char* path) -> bool {
  const auto ret = ::DeleteFileA(path);
  return ret != FALSE;
}

static inline auto rename(const char* old_path, const char* new_path) -> bool {
  const auto ret = ::MoveFileA(old_path, new_path);
  return ret != FALSE;
}

static inline auto mkdir(const char* path) -> bool {
  const auto ret = ::CreateDirectoryA(path, nullptr);
  return ret != FALSE;
}

static inline auto rmdir(const char* path) -> bool {
  const auto ret = ::RemoveDirectoryA(path);
  return ret != FALSE;
}

}  // namespace sfc::sys::io
