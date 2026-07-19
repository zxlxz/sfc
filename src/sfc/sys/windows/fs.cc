#include <Windows.h>
#undef min
#undef max

#include "sfc/sys/windows/fs.h"
#include "sfc/fs/file.h"

namespace sfc::sys::windows {

auto FileAttr::is_dir() const -> bool {
  return _attr & FILE_ATTRIBUTE_DIRECTORY;
}

auto FileAttr::is_file() const -> bool {
  return _attr & FILE_ATTRIBUTE_NORMAL;
}

auto open(const wchar_t* path, OpenOptions opts) -> io::Result<RawFd> {
  const DWORD read_mode = opts.read ? GENERIC_READ : 0U;
  const DWORD write_mode = opts.write ? GENERIC_WRITE : 0U;
  const DWORD append_mode = opts.append ? FILE_APPEND_DATA : 0U;
  const DWORD access_mode = read_mode | write_mode | append_mode;

  const DWORD create_mode = opts.create_new           ? CREATE_NEW
                            : opts.create && opts.truncate ? CREATE_ALWAYS
                            : opts.create               ? OPEN_ALWAYS
                            : opts.truncate             ? TRUNCATE_EXISTING
                                                        : OPEN_EXISTING;

  const DWORD share_mode = FILE_SHARE_READ | FILE_SHARE_WRITE;
  const DWORD flags = FILE_ATTRIBUTE_NORMAL;

  const auto handle =
      ::CreateFileW(path, access_mode, share_mode, nullptr, create_mode, flags, nullptr);
  if (handle == INVALID_HANDLE_VALUE) {
    return io::last_os_error();
  }
  return handle;
}

auto lstat(const wchar_t* path) -> io::Result<Metadata> {
  auto attr = WIN32_FILE_ATTRIBUTE_DATA{};
  if (!::GetFileAttributesExW(path, GetFileExInfoStandard, &attr)) {
    return io::last_os_error();
  }

  const auto size = (u64{attr.nFileSizeHigh} << 32) | u64{attr.nFileSizeLow};
  const auto meta = Metadata{attr.dwFileAttributes, size};
  return Ok{meta};
}

auto unlink(const wchar_t* path) -> io::Result<> {
  if (auto ret = ::DeleteFileW(path); !ret) {
    return io::last_os_error();
  }
  return Ok{};
}

auto rename(const wchar_t* old_path, const wchar_t* new_path) -> io::Result<> {
  if (auto ret = ::MoveFileW(old_path, new_path); !ret) {
    return io::last_os_error();
  }
  return Ok{};
}

auto mkdir(const wchar_t* path) -> io::Result<> {
  if (auto ret = ::CreateDirectoryW(path, nullptr); !ret) {
    return io::last_os_error();
  }
  return Ok{};
}

auto rmdir(const wchar_t* path) -> io::Result<> {
  if (auto ret = ::RemoveDirectoryW(path); !ret) {
    return io::last_os_error();
  }
  return Ok{};
}

}  // namespace sfc::sys::windows
