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

  auto is_symlink() const -> bool {
    return S_ISLNK(_mode);
  }
};

inline auto lstat(cstr_t path) -> Option<FileType> {
  struct stat st;
  const auto ret = ::lstat(path, &st);
  if (ret == -1) {
    return {};
  }
  return FileType{st.st_mode, st.st_size};
};

inline auto unlink(cstr_t path) -> bool {
  return ::unlink(path) == 0;
}

inline auto rename(cstr_t old_path, cstr_t new_path) -> bool {
  return ::rename(old_path, new_path) == 0;
}

inline auto mkdir(cstr_t path) -> bool {
  const mode_t mode = 0777;
  return ::mkdir(path, mode) == 0;
}

inline auto rmdir(cstr_t path) -> bool {
  return ::rmdir(path) == 0;
}

inline auto symlink(cstr_t original, cstr_t link) -> bool {
  return ::symlink(original, link) == 0;
}

}  // namespace sfc::sys::fs
