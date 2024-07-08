#pragma once

#include <fcntl.h>
#include <sys/stat.h>

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

  // sys
  int _mode = 0666;

 public:
  OpenOptions(const auto& x)
      : _append{x._append}
      , _create{x._create}
      , _create_new{x._create_new}
      , _read{x._read}
      , _write{x._write}
      , _truncate{x._truncate} {}

  auto access_mode() const -> int {
    const auto a = (_write || _append) ? _read ? O_RDWR : O_WRONLY : O_RDONLY;
    const auto b = _append ? O_APPEND : 0;
    return a | b;
  }

  auto create_mode() const -> int {
    const auto a = (_create || _create_new) ? O_CREAT : 0;
    const auto b = _create_new ? O_EXCL : 0;
    const auto c = _truncate ? O_TRUNC : 0;
    return a | b | c;
  }

  auto open(const char* path) const -> io::File {
    const auto flags = this->access_mode() | this->create_mode();
    const auto fd = ::open(path, O_CLOEXEC | flags, _mode);
    return io::File{fd};
  }
};

struct FileType {
  mode_t _mode;
  off_t _size;

 public:
  auto is_dir() const -> bool {
    return S_ISDIR(_mode);
  }

  auto is_file() const -> bool {
    return S_ISREG(_mode);
  }

  auto is_link() const -> bool {
    return S_ISLNK(_mode);
  }
};

auto lstat(cstr_t path) -> FileType {
  struct stat st;
  const auto ret = ::lstat(path, &st);
  if (ret == -1) {
    return {0, 0};
  }
  return {st.st_mode, st.st_size};
};

}  // namespace sfc::sys::fs
