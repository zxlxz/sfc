#pragma once

#include "sfc/io/mod.h"
#include "sfc/sys.h"

namespace sfc::io {

class File {
  sys::File _inn;

 public:
  explicit File(sys::File inn = {}) noexcept;
  ~File() noexcept;

  File(File&&) noexcept;
  File& operator=(File&&) noexcept;

 public:
  auto as_raw_fd() const noexcept -> sys::RawFd;
  auto is_valid() const noexcept -> bool;

  auto read(Slice<u8> buf) noexcept -> Result<usize>;
  auto write(Slice<const u8> buf) noexcept -> Result<usize>;
  auto seek(SeekFrom pos) noexcept -> Result<usize>;
  auto flush() noexcept -> Result<>;

  auto is_tty() const noexcept -> bool;
};

}  // namespace sfc::io
