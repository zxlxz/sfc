#pragma once
#ifdef _WIN32
#include <Windows.h>

namespace sfc::sys::fs {

static const auto INVALID_FD = INVALID_HANDLE_VALUE;

static inline auto combine_u64(DWORD high, DWORD low) -> SIZE_T {
  return (static_cast<SIZE_T>(high) << 32) | static_cast<SIZE_T>(low);
}

static inline auto open(const char* path, const auto& opts) -> HANDLE {
  const auto access_mode = (opts.read ? GENERIC_READ : 0) |    //
                           (opts.write ? GENERIC_WRITE : 0) |  //
                           (opts.append ? FILE_APPEND_DATA : 0);

  const auto share_mode = FILE_SHARE_READ | FILE_SHARE_WRITE;

  const auto create_mode = opts.create_new                ? CREATE_NEW
                           : opts.create && opts.truncate ? CREATE_ALWAYS
                           : opts.create                  ? OPEN_ALWAYS
                           : opts.truncate                ? TRUNCATE_EXISTING
                                                          : OPEN_EXISTING;

  const auto flags = FILE_ATTRIBUTE_NORMAL;

  const auto handle =
      ::CreateFileA(path, access_mode, share_mode, nullptr, create_mode, flags, nullptr);

  return handle;
}

static inline auto is_dir(DWORD attr) -> bool {
  return attr & FILE_ATTRIBUTE_DIRECTORY;
}

static inline auto is_file(DWORD attr) -> bool {
  return attr & FILE_ATTRIBUTE_NORMAL;
}

static inline bool lstat(const char* path, auto& res) {
  auto attr = WIN32_FILE_ATTRIBUTE_DATA{};
  if (!::GetFileAttributesExA(path, GetFileExInfoStandard, &attr)) {
    return false;
  }

  const auto size = combine_u64(attr.nFileSizeHigh, attr.nFileSizeLow);

  res._attr = static_cast<UINT32>(attr.dwFileAttributes);
  res._size = size;
  return true;
}

static inline auto unlink(const char* path) -> bool {
  return ::DeleteFileA(path);
}

static inline auto rename(const char* old_path, const char* new_path) -> bool {
  return ::MoveFileA(old_path, new_path);
}

static inline auto mkdir(const char* path) -> bool {
  return ::CreateDirectoryA(path, nullptr);
}

static inline auto rmdir(const char* path) -> bool {
  return ::RemoveDirectoryA(path);
}

}  // namespace sfc::sys::fs
#endif
