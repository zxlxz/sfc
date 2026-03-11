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

  auto open(const wchar_t* path) const -> File {
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
    return File{handle};
  }
};

static inline auto is_dir(DWORD attr) -> bool {
  return attr & FILE_ATTRIBUTE_DIRECTORY;
}

static inline auto is_file(DWORD attr) -> bool {
  return attr & FILE_ATTRIBUTE_NORMAL;
}

static inline bool lstat(const wchar_t* path, auto& res) {
  auto attr = WIN32_FILE_ATTRIBUTE_DATA{};
  if (!::GetFileAttributesExW(path, GetFileExInfoStandard, &attr)) {
    return false;
  }

  const auto size = (SIZE_T(attr.nFileSizeHigh) << 32U) | attr.nFileSizeLow;
  res._attr = static_cast<UINT32>(attr.dwFileAttributes);
  res._size = size;
  return true;
}

static inline auto unlink(const wchar_t* path) -> bool {
  return ::DeleteFileW(path);
}

static inline auto rename(const wchar_t* old_path, const wchar_t* new_path) -> bool {
  return ::MoveFileW(old_path, new_path);
}

static inline auto mkdir(const wchar_t* path) -> bool {
  return ::CreateDirectoryW(path, nullptr);
}

static inline auto rmdir(const wchar_t* path) -> bool {
  return ::RemoveDirectoryW(path);
}

}  // namespace sfc::sys::windows
