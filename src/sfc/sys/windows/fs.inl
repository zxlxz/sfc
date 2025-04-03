#pragma once

#include <windows.h>

#include "io.inl"

namespace sfc::sys::fs {

struct OpenOptions {
  // user
  bool _append;
  bool _create;
  bool _create_new;
  bool _read;
  bool _write;
  bool _truncate;

 public:
  OpenOptions(const auto& x)
      : _append{x._append},
        _create{x._create},
        _create_new{x._create_new},
        _read{x._read},
        _write{x._write},
        _truncate{x._truncate} {}

  auto open(const char* path) const -> io::File {
    const auto access_mode = _append ? FILE_APPEND_DATA : _write ? GENERIC_WRITE : GENERIC_READ;
    const auto share_mode  = FILE_SHARE_READ;
    const auto create_mode = _create_new ? CREATE_NEW : _write ? CREATE_ALWAYS : OPEN_EXISTING;
    const auto file_attrs  = FILE_ATTRIBUTE_NORMAL;
    const auto handle =
        ::CreateFileA(path, access_mode, share_mode, nullptr, create_mode, file_attrs, nullptr);

    return io::File{handle};
  }
};

struct FileAttr {
  DWORD _attr;
  LONG64 _size;

 public:
 inline auto is_dir() const -> bool {
    return _attr & FILE_ATTRIBUTE_DIRECTORY;
  }

  inline auto is_file() const -> bool {
    return _attr & FILE_ATTRIBUTE_NORMAL;
  }
};

inline auto lstat(cstr_t path) -> Option<FileAttr> {
  auto attr = WIN32_FILE_ATTRIBUTE_DATA{};
  if (GetFileAttributesExA(path, GetFileExInfoStandard, &attr) == 0) {
    return {};
  }

  const auto file_size = LONG64(attr.nFileSizeHigh) + (LONG64(attr.nFileSizeLow) << 32);
  return FileAttr{attr.dwFileAttributes, file_size};
};

inline auto unlink(cstr_t path) -> bool {
  const auto ret = ::DeleteFileA(path);
  return ret != 0;
}

inline auto rename(cstr_t old_path, cstr_t new_path) -> bool {
  const auto ret = ::MoveFileA(old_path, new_path);
  return ret != 0;
}

inline auto mkdir(cstr_t path) -> bool {
  const auto ret = ::CreateDirectoryA(path, nullptr);
  return ret != 0;
}

inline auto rmdir(cstr_t path) -> bool {
  const auto ret = ::RemoveDirectoryA(path);
  return ret != 0;
}

}  // namespace sfc::sys::fs
