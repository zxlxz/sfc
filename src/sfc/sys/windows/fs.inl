#pragma once
#include "sfc/sys/windows/mod.inl"

namespace sfc::sys::fs {

static const auto INVALID_FD = INVALID_HANDLE_VALUE;

static inline auto open(const wchar_t* path, const auto& opts) -> HANDLE {
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
  const auto handle = ::CreateFileW(path, access_mode, share_mode, nullptr, create_mode, flags, nullptr);

  return handle;
}

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

}  // namespace sfc::sys::fs
