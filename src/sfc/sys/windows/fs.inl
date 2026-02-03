#pragma once
#ifdef _WIN32
#include <Windows.h>

namespace sfc::sys::fs {

static const auto INVALID_FD = INVALID_HANDLE_VALUE;

static inline auto combine_u64(DWORD high, DWORD low) -> SIZE_T {
  return (static_cast<SIZE_T>(high) << 32) | static_cast<SIZE_T>(low);
}

template <int N>
static inline auto to_os_str(const char src[], wchar_t (&dst)[N]) -> bool {
  const auto ret = ::MultiByteToWideChar(CP_UTF8, 0, src, -1, dst, N);
  return ret >= 0;
}

static inline auto open(const char* path, const auto& opts) -> HANDLE {
  wchar_t os_path[MAX_PATH];
  if (to_os_str(path, os_path) == 0) {
    return nullptr;
  }

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
  const auto handle = ::CreateFileW(os_path, access_mode, share_mode, nullptr, create_mode, flags, nullptr);

  return handle;
}

static inline auto is_dir(DWORD attr) -> bool {
  return attr & FILE_ATTRIBUTE_DIRECTORY;
}

static inline auto is_file(DWORD attr) -> bool {
  return attr & FILE_ATTRIBUTE_NORMAL;
}

static inline bool lstat(const char* path, auto& res) {
  wchar_t os_path[MAX_PATH] = {};
  if (!to_os_str(path, os_path)) {
    return false;
  }

  auto attr = WIN32_FILE_ATTRIBUTE_DATA{};
  if (!::GetFileAttributesExW(os_path, GetFileExInfoStandard, &attr)) {
    return false;
  }

  const auto size = combine_u64(attr.nFileSizeHigh, attr.nFileSizeLow);

  res._attr = static_cast<UINT32>(attr.dwFileAttributes);
  res._size = size;
  return true;
}

static inline auto unlink(const char* path) -> bool {
  wchar_t os_path[MAX_PATH] = {};
  if (!to_os_str(path, os_path)) {
    return false;
  }

  return ::DeleteFileW(os_path);
}

static inline auto rename(const char* old_path, const char* new_path) -> bool {
  wchar_t old_os_path[MAX_PATH] = {};
  if (!to_os_str(old_path, old_os_path)) {
    return false;
  }

  wchar_t new_os_path[MAX_PATH] = {};
  if (!to_os_str(new_path, new_os_path)) {
    return false;
  }

  return ::MoveFileW(old_os_path, new_os_path);
}

static inline auto mkdir(const char* path) -> bool {
  wchar_t os_path[MAX_PATH] = {};
  if (!to_os_str(path, os_path)) {
    return false;
  }
  return ::CreateDirectoryW(os_path, nullptr);
}

static inline auto rmdir(const char* path) -> bool {
  wchar_t os_path[MAX_PATH] = {};
  if (to_os_str(path, os_path) == 0) {
    return false;
  }
  return ::RemoveDirectoryW(os_path);
}

}  // namespace sfc::sys::fs
#endif
