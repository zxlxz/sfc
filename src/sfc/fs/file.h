#pragma once

#include "sfc/sys.h"
#include "sfc/io/mod.h"
#include "sfc/fs/path.h"

namespace sfc::fs {

class [[nodiscard]] File : public io::Read, public io::Write {
  sys::File _inn;

 public:
  File() noexcept;
  ~File() noexcept;
  File(File&&) noexcept;
  File& operator=(File&&) noexcept;

  static auto from_raw_fd(sys::RawFd fd) -> File;
  static auto open(Path path) noexcept -> io::Result<File>;
  static auto create(Path path) noexcept -> io::Result<File>;

  auto is_valid() const -> bool;
  auto read(Slice<u8> buf) noexcept -> io::Result<usize>;
  auto write(Slice<const u8> buf) noexcept -> io::Result<usize>;
  auto flush() -> io::Result<>;
  auto seek(io::SeekFrom pos) noexcept -> io::Result<usize>;
};

struct OpenOptions {
  bool append = false;
  bool create = false;
  bool create_new = false;
  bool read = false;
  bool write = false;
  bool truncate = false;

 public:
  auto open(Path path) const noexcept -> io::Result<File>;
};

auto read(Path path) noexcept -> io::Result<List<u8>>;
auto write(Path path, Slice<const u8> buf) noexcept -> io::Result<>;

}  // namespace sfc::fs

namespace sfc::io {

extern template auto Read::read_exact(this fs::File&, Slice<u8>) -> Result<>;
extern template auto Read::read_to_end(this fs::File&, List<u8>&) -> Result<usize>;
extern template auto Read::read_to_string(this fs::File&, String&) -> Result<usize>;

extern template auto Write::write_all(this fs::File&, Slice<const u8>) -> Result<>;
extern template auto Write::write_str(this fs::File&, Str) -> Result<>;
}  // namespace sfc::io
