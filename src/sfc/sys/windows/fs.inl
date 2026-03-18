#pragma once

#include "sfc/sys/windows/io.inl"

namespace sfc::sys::windows {

struct File {
  HANDLE _fd;

 public:
  File(HANDLE fd = nullptr) noexcept : _fd(fd) {}

  ~File() {
    if (_fd == nullptr || _fd == INVALID_HANDLE_VALUE) return;
    ::CloseHandle(_fd);
  }

  File(File&& other) noexcept : _fd(other._fd) {
    other._fd = nullptr;
  }

  File& operator=(File&& other) noexcept {
    if (this != &other) {
      mem::swap(_fd, other._fd);
    }
    return *this;
  }

  auto is_valid() const -> bool {
    return _fd != nullptr && _fd != INVALID_HANDLE_VALUE;
  }

  auto is_terminal() const -> bool {
    auto mode = 0UL;
    if (!::GetConsoleMode(_fd, &mode)) {
      return false;
    }
    return mode != 0;
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

struct OpenOptions {
  bool _append = false;
  bool _create = false;
  bool _create_new = false;
  bool _read = false;
  bool _write = false;
  bool _truncate = false;
  DWORD _share_mode = FILE_SHARE_READ | FILE_SHARE_WRITE;
  DWORD _flags = FILE_ATTRIBUTE_NORMAL;

 public:
  auto open(const wchar_t* path) const -> io::Result<HANDLE> {
    const auto read_mode = _read ? GENERIC_READ : 0;
    const auto write_mode = _write ? GENERIC_WRITE : 0;
    const auto append_mode = _append ? FILE_APPEND_DATA : 0;
    const auto access_mode = read_mode | write_mode | append_mode;

    const auto create_mode = _create_new            ? CREATE_NEW
                             : _create && _truncate ? CREATE_ALWAYS
                             : _create              ? OPEN_ALWAYS
                             : _truncate            ? TRUNCATE_EXISTING
                                                    : OPEN_EXISTING;

    const auto handle = ::CreateFileW(path, access_mode, _share_mode, nullptr, create_mode, _flags, nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
      return io::last_os_error();
    }
    return handle;
  }
};

struct FileAttr {
  DWORD _attr = 0;

  auto is_dir() const -> bool {
    return _attr & FILE_ATTRIBUTE_DIRECTORY;
  }

  auto is_file() const -> bool {
    return _attr & FILE_ATTRIBUTE_NORMAL;
  }
};

struct Metadata {
  DWORD _attr;
  SIZE_T _size;

 public:
  auto is_dir() const -> bool {
    return _attr & FILE_ATTRIBUTE_DIRECTORY;
  }

  auto is_file() const -> bool {
    return _attr & FILE_ATTRIBUTE_NORMAL;
  }
};

static inline auto lstat(const wchar_t* path) -> io::Result<Metadata> {
  auto attr = WIN32_FILE_ATTRIBUTE_DATA{};
  if (!::GetFileAttributesExW(path, GetFileExInfoStandard, &attr)) {
    return io::last_os_error();
  }

  const auto size = (SIZE_T(attr.nFileSizeHigh) << 32U) | attr.nFileSizeLow;
  return Metadata{attr.dwFileAttributes, size};
}

static inline auto unlink(const wchar_t* path) -> io::Result<> {
  const auto ret = ::DeleteFileW(path);
  if (!ret) {
    return io::last_os_error();
  }
  return {};
}

static inline auto rename(const wchar_t* old_path, const wchar_t* new_path) -> io::Result<> {
  const auto ret = ::MoveFileW(old_path, new_path);
  if (!ret) {
    return io::last_os_error();
  }
  return {};
}

static inline auto mkdir(const wchar_t* path) -> io::Result<> {
  const auto ret = ::CreateDirectoryW(path, nullptr);
  if (!ret) {
    return io::last_os_error();
  }
  return {};
}

static inline auto rmdir(const wchar_t* path) -> io::Result<> {
  const auto ret = ::RemoveDirectoryW(path);
  if (!ret) {
    return io::last_os_error();
  }
  return {};
}

}  // namespace sfc::sys::windows
