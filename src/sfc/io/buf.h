#pragma once

#include "sfc/io/mod.h"

namespace sfc::io {

struct BufReader : IRead {
  Read _inn;
  Slice<u8> _buf;
  usize _start{0};
  usize _end{0};

 public:
  static auto new_(Read inn, Slice<u8> buf) -> BufReader;

 public:
  auto capacity() const -> usize;
  auto has_data_left() const -> bool;
  auto buffer() const -> Slice<const u8>;

  void backshift();
  void discard_buffer();
  void consume(usize amount);

 public:
  auto fill_buf() -> Result<Slice<const u8>>;
  auto skip_until(u8 byte) -> Result<usize>;

  auto read(Slice<u8> buf) -> Result<usize>;
  auto read_until(u8 byte, List<u8>& buf) -> Result<usize>;
  auto read_line(String& buf) -> Result<usize>;
};

struct BufWriter : IWrite {
  Write _inn;
  Slice<u8> _buf;
  usize _end{0};

 public:
  static auto new_(Write inn, Slice<u8> buf) -> BufWriter;

 public:
  auto capacity() const -> usize;
  auto buffer() const -> Slice<const u8>;
  auto spare_capacity_mut() -> Slice<u8>;

 public:
  auto write(Slice<const u8> buf) -> Result<usize>;
  auto flush() -> Result<>;
};

}  // namespace sfc::io
