#pragma once

#include "error.h"

namespace sfc::io {

class File {
 protected:
  int _fd = -1;

  File(int fd) noexcept;

 public:
  File() noexcept;
  ~File();
  File(File&&) noexcept;
  File& operator=(File&&) noexcept;

  static auto from_raw(int fd) -> File;

 public:
  explicit operator bool() const;

  auto read(Slice<u8> buf) -> usize;
  auto write(Slice<const u8> buf) -> usize;

  auto read_all(Vec<u8>& buf, usize buf_len = 256) -> usize;
  auto read_to_string(String& buf) -> usize;

  auto write_all(Slice<const u8> buf) -> usize;
  auto write_str(Str str) -> usize;

  auto write_fmt(Str pattern, const auto&... args) -> usize {
    const auto s = string::format(pattern, args...);
    const auto n = this->write_str(s);
    return n;
  }
};

}  // namespace sfc::io
