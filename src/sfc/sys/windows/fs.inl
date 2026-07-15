#pragma once

#include "sfc/sys/windows/io.inl"

namespace sfc::sys::windows {

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
    const DWORD read_mode = _read ? GENERIC_READ : 0U;
    const DWORD write_mode = _write ? GENERIC_WRITE : 0U;
    const DWORD append_mode = _append ? FILE_APPEND_DATA : 0U;
    const DWORD access_mode = read_mode | write_mode | append_mode;
    const DWORD create_mode = _create_new            ? CREATE_NEW
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
  const auto meta = Metadata{attr.dwFileAttributes, size};
  return Ok{meta};
}

static inline auto unlink(const wchar_t* path) -> io::Result<> {
  if (auto ret = ::DeleteFileW(path); !ret) {
    return io::last_os_error();
  }
  return Ok{};
}

static inline auto rename(const wchar_t* old_path, const wchar_t* new_path) -> io::Result<> {
  if (auto ret = ::MoveFileW(old_path, new_path); !ret) {
    return io::last_os_error();
  }
  return Ok{};
}

static inline auto mkdir(const wchar_t* path) -> io::Result<> {
  if (auto ret = ::CreateDirectoryW(path, nullptr); !ret) {
    return io::last_os_error();
  }
  return Ok{};
}

static inline auto rmdir(const wchar_t* path) -> io::Result<> {
  if (auto ret = ::RemoveDirectoryW(path); !ret) {
    return io::last_os_error();
  }
  return Ok{};
}

}  // namespace sfc::sys::windows
