#include <Windows.h>

#include "sfc/fs/file.h"

#include "sfc/sys/windows/io.h"
#include "sfc/sys/windows/fs.h"
#include "sfc/sys/windows/wstr.h"

#define _CVT_R(expr)                              \
  ({                                              \
    auto ret = (expr);                            \
    if (ret == FALSE) return io::last_os_error(); \
    ret;                                          \
  })

namespace sfc::sys::windows {

inline auto combine_u64(u32 high, u32 low) -> u64 {
  return (u64(high) << 32) | u64(low);
}

auto open(Str path, fs::OpenOptions opts) -> io::Result<RawFd> {
  const auto os_path = FixedWStr<kMaxPath>::from_str(path);
  if (!os_path) {
    return io::Error::InvalidInput;
  }

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

  const auto handle = ::CreateFileW(os_path, access_mode, share_mode, nullptr, create_mode, flags, nullptr);
  _CVT_R(handle != INVALID_HANDLE_VALUE);

  return handle;
}

auto fstat(RawFd fd) -> io::Result<fs::Metadata> {
  auto attr = BY_HANDLE_FILE_INFORMATION{};

  _CVT_R(::GetFileInformationByHandle(fd, &attr));

  const auto size = combine_u64(attr.nFileSizeHigh, attr.nFileSizeLow);
  const auto meta = fs::Metadata{._size = size, ._attr = attr.dwFileAttributes};
  return Ok{meta};
}

auto lstat(Str path) -> io::Result<fs::Metadata> {
  const auto os_path = FixedWStr<kMaxPath>::from_str(path);
  if (!os_path) {
    return io::Error::InvalidInput;
  }

  auto attr = WIN32_FILE_ATTRIBUTE_DATA{};
  _CVT_R(::GetFileAttributesExW(os_path, GetFileExInfoStandard, &attr));

  const auto size = combine_u64(attr.nFileSizeHigh, attr.nFileSizeLow);
  const auto meta = fs::Metadata{._size = size, ._attr = attr.dwFileAttributes};
  return Ok{meta};
}

auto unlink(Str path) -> io::Result<> {
  const auto os_path = FixedWStr<kMaxPath>::from_str(path);
  if (!os_path) {
    return io::Error::InvalidInput;
  }

  _CVT_R(::DeleteFileW(os_path));
  return Ok{};
}

auto rename(Str old_path, Str new_path) -> io::Result<> {
  const auto os_old_path = FixedWStr<kMaxPath>::from_str(old_path);
  const auto os_new_path = FixedWStr<kMaxPath>::from_str(new_path);
  if (!os_old_path || !os_new_path) {
    return io::Error::InvalidInput;
  }

  _CVT_R(::MoveFileW(os_old_path, os_new_path));
  return Ok{};
}

auto mkdir(Str path) -> io::Result<> {
  const auto os_path = FixedWStr<kMaxPath>::from_str(path);
  if (!os_path) {
    return io::Error::InvalidInput;
  }

  _CVT_R(::CreateDirectoryW(os_path, nullptr));

  return Ok{};
}

auto rmdir(Str path) -> io::Result<> {
  const auto os_path = FixedWStr<kMaxPath>::from_str(path);
  if (!os_path) {
    return io::Error::InvalidInput;
  }

  _CVT_R(::RemoveDirectoryW(os_path));
  return Ok{};
}

auto FileAttr::is_dir() const noexcept -> bool {
  return (_attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

auto FileAttr::is_file() const noexcept -> bool {
  return (_attr & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

auto FileAttr::is_link() const noexcept -> bool {
  return (_attr & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
}

auto ReadDir::open(Str path) -> io::Result<ReadDir> {
  const auto os_path = FixedWStr<kMaxPath>::from_str(path);
  if (!os_path) {
    return io::Error::InvalidInput;
  }

  wchar_t os_pattern[kMaxPath];
  wcscpy_s(os_pattern, kMaxPath, os_path._buf);
  wcscat_s(os_pattern, kMaxPath, L"\\*");

  WIN32_FIND_DATAW data{};
  const auto handle = ::FindFirstFileW(os_pattern, &data);
  if (handle == INVALID_HANDLE_VALUE) {
    const auto os_err = ::GetLastError();
    if (os_err == ERROR_FILE_NOT_FOUND) {
      return ReadDir{};  // empty dir
    }
    return io_error(os_err);
  }

  auto res = ReadDir{
      ._handle = handle,
      ._file_size = combine_u64(data.nFileSizeHigh, data.nFileSizeLow),
      ._file_attr = data.dwFileAttributes,
      ._file_name = {},
  };
  windows::cvt_w16_utf8(data.cFileName, res._file_name);

  return res;
}

auto ReadDir::close() -> io::Result<> {
  if (_handle != nullptr) {
    ::FindClose(_handle);
  }

  return Ok{};
}

auto ReadDir::next() -> Option<DirEntry> {
  if (_handle == nullptr || _handle == INVALID_HANDLE_VALUE) {
    return option::None{};
  }

  if (_index != 0) {
    auto dataw = WIN32_FIND_DATAW{};
    if (!::FindNextFileW(_handle, &dataw)) {
      return option::None{};
    }

    _file_size = combine_u64(dataw.nFileSizeHigh, dataw.nFileSizeLow);
    _file_attr = dataw.dwFileAttributes;
    windows::cvt_w16_utf8(dataw.cFileName, _file_name);
  }

  return DirEntry{
      ._name = _file_name,
      ._size = _file_size,
      ._attr = _file_attr,
  };
}

}  // namespace sfc::sys::windows
