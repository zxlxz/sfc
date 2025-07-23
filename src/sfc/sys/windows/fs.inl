#pragma once

#include <Windows.h>

namespace sfc::sys::fs {

static inline bool open(const auto& opts, const char* path, HANDLE* fd) {
  const auto access_mode = (opts.read ? GENERIC_READ : 0) | (opts.write ? GENERIC_WRITE : 0);
  const auto share_mode = opts.read ? FILE_SHARE_READ : 0;
  const auto create_mode = opts.create_new ? CREATE_NEW : opts.create ? OPEN_ALWAYS : OPEN_EXISTING;
  const auto flags = opts.append ? FILE_FLAG_OVERLAPPED : 0;

  const auto handle = ::CreateFileA(path,
                                    access_mode,
                                    share_mode,
                                    nullptr,
                                    create_mode,
                                    flags | FILE_ATTRIBUTE_NORMAL,
                                    nullptr);

  if (handle == INVALID_HANDLE_VALUE) {
    return false;
  }

  *fd = handle;
  return true;
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
