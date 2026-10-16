#include <Windows.h>

#include "sfc/fs/file.h"
#include "sfc/sys/windows/fs.h"

namespace sfc::sys::windows {

auto FileAttr::exists() const noexcept -> bool {
  return _attr != 0;
}

auto FileAttr::is_dir() const noexcept -> bool {
  return (_attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

auto FileAttr::is_file() const noexcept -> bool {
  // not dir
  return (_attr & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

auto FileAttr::is_link() const noexcept -> bool {
  return (_attr & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
}

auto DirEntry::attr() const -> u32 {
  return _attr;
}

auto DirEntry::path() const -> ffi::WStr {
  const auto len = wcslen(_path);
  return ffi::WStr{_path, len};
}

auto open(const wchar_t* path, fs::OpenOptions opts) -> io::Result<RawFd> {
  const DWORD read_mode = opts.read ? GENERIC_READ : 0U;
  const DWORD write_mode = opts.write ? GENERIC_WRITE : 0U;
  const DWORD append_mode = opts.append ? FILE_APPEND_DATA : 0U;
  const DWORD access_mode = read_mode | write_mode | append_mode;

  const DWORD create_mode = opts.create_new                ? CREATE_NEW
                            : opts.create && opts.truncate ? CREATE_ALWAYS
                            : opts.create                  ? OPEN_ALWAYS
                            : opts.truncate                ? TRUNCATE_EXISTING
                                                           : OPEN_EXISTING;

  const DWORD share_mode = FILE_SHARE_READ | FILE_SHARE_WRITE;
  const DWORD flags = FILE_ATTRIBUTE_NORMAL;

  const auto handle = ::CreateFileW(path, access_mode, share_mode, nullptr, create_mode, flags, nullptr);
  if (handle == INVALID_HANDLE_VALUE) {
    return io::last_os_error();
  }
  return handle;
}

auto fstat(void* fd) -> io::Result<fs::Metadata> {
  auto attr = BY_HANDLE_FILE_INFORMATION{};
  if (!::GetFileInformationByHandle(fd, &attr)) {
    return io::last_os_error();
  }

  const auto size = (u64{attr.nFileSizeHigh} << 32) | u64{attr.nFileSizeLow};
  const auto meta = fs::Metadata{attr.dwFileAttributes, size};
  return Ok{meta};
}

auto lstat(const wchar_t* path) -> io::Result<fs::Metadata> {
  auto attr = WIN32_FILE_ATTRIBUTE_DATA{};
  if (!::GetFileAttributesExW(path, GetFileExInfoStandard, &attr)) {
    return io::last_os_error();
  }

  const auto size = (u64{attr.nFileSizeHigh} << 32) | u64{attr.nFileSizeLow};
  const auto meta = fs::Metadata{attr.dwFileAttributes, size};
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

auto open_dir(const wchar_t* path) -> io::Result<ReadDir*> {
  wchar_t pattern[MAX_PATH + 4] = {};
  wcscat_s(pattern, path);
  wcscat_s(pattern, L"\\*");

  WIN32_FIND_DATAW data{};
  const auto handle = ::FindFirstFileW(pattern, &data);
  if (handle == INVALID_HANDLE_VALUE) {
    return io::last_os_error();
  }

  auto res = new ReadDir{
      .handle = handle,
      .entry = {._path = {}, ._attr = data.dwFileAttributes},
  };
  wcscpy_s(res->entry._path, pattern);
  return Ok{res};
}

auto close_dir(ReadDir* dir) -> io::Result<> {
  if (dir == nullptr) {
    return Ok{};
  }

  if (dir->handle != nullptr) {
    ::FindClose(dir->handle);
  }

  delete dir;

  return Ok{};
}

auto read_dir(ReadDir* dir) -> Option<DirEntry> {
  if (dir == nullptr) {
    return {};
  }
  if (dir->handle == nullptr || dir->handle == INVALID_HANDLE_VALUE) {
    return {};
  }

  WIN32_FIND_DATAW data{};
  if (!::FindNextFileW(dir->handle, &data)) {
    return {};
  }

  auto entry = DirEntry{._path = {}, ._attr = data.dwFileAttributes};
  wcscat_s(entry._path, data.cFileName);
  return entry;
}

}  // namespace sfc::sys::windows
