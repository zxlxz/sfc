#pragma once

#include "sfc/io/mod.h"

namespace sfc::io {

class [[nodiscard]] File {
#ifdef _WIN32
  using fd_t = void*;
#else
  using fd_t = int;
#endif
  fd_t _fd;

 public:
  explicit File() noexcept;
  ~File() noexcept;

  File(File&&) noexcept;
  File& operator=(File&&) noexcept;

  File(const File&) = delete;
  File& operator=(const File&) = delete;

  static auto from_fd(fd_t fd) -> File;

  auto as_fd() const -> fd_t;
  auto is_open() const -> bool;
  void close();

  auto read(Slice<u8> buf) -> Result<usize>;
  auto read_to_end(Vec<u8>& buf, usize buf_len = 256) -> Result<usize>;
  auto read_to_string(String& buf, usize buf_len = 256) -> Result<usize>;

  auto write(Slice<const u8> buf) -> Result<usize>;
  auto write_all(Slice<const u8> buf) -> Result<usize>;
  auto write_str(Str str) -> io::Result<usize>;
};

}  // namespace sfc::io
