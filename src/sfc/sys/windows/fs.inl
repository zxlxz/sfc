#pragma once

#include <Windows.h>

namespace sfc::sys::fs {

static inline auto null() -> HANDLE {
  return nullptr;
}

static inline auto open(const char* path, const auto& opts) -> HANDLE {
  const auto access_mode = (opts.read ? GENERIC_READ : 0) | (opts.write ? GENERIC_WRITE : 0) |
                           (opts.append ? FILE_APPEND_DATA : 0);

  auto create_mode = 0U;
  if (opts.create_new) {
    create_mode = CREATE_NEW;
  } else if (opts.create) {
    create_mode = opts.truncate ? CREATE_ALWAYS : OPEN_ALWAYS;
  } else {
    create_mode = opts.truncate ? TRUNCATE_EXISTING : OPEN_EXISTING;
  }

  // File attributes
  const auto flags = FILE_ATTRIBUTE_NORMAL;

  // Call CreateFileA and return the handle (ignore failures)
  return ::CreateFileA(path, access_mode, opts.share_mode, nullptr, create_mode, flags, nullptr);
}

static inline auto is_dir(DWORD attr) -> bool {
  return attr & FILE_ATTRIBUTE_DIRECTORY;
}

static inline auto is_file(DWORD attr) -> bool {
  return attr & FILE_ATTRIBUTE_NORMAL;
}

static inline bool lstat(const char* path, auto& res) {
  auto attr = WIN32_FILE_ATTRIBUTE_DATA{};
  if (!GetFileAttributesExA(path, GetFileExInfoStandard, &attr)) {
    return false;
  }

  const auto size = static_cast<SIZE_T>(attr.nFileSizeHigh) +
                    (static_cast<SIZE_T>(attr.nFileSizeLow) << 32);

  res._attr = static_cast<UINT32>(attr.dwFileAttributes);
  res._size = size;
  return true;
}

static inline auto unlink(const char* path) -> bool {
  return DeleteFileA(path);
}

static inline auto rename(const char* old_path, const char* new_path) -> bool {
  return MoveFileA(old_path, new_path);
}

static inline auto mkdir(const char* path) -> bool {
  return CreateDirectoryA(path, nullptr);
}

static inline auto rmdir(const char* path) -> bool {
  return RemoveDirectoryA(path);
}

}  // namespace sfc::sys::fs
