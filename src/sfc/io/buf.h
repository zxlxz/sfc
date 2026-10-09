#pragma once

#include "sfc/io/mod.h"

namespace sfc::io {

class ReadBuf {
  List<u8> _buf{};
  usize _pos = 0;

 public:
  ReadBuf();
  ~ReadBuf();
  ReadBuf(ReadBuf&&) noexcept;
  ReadBuf& operator=(ReadBuf&&) noexcept;

  static auto with_capacity(usize capacity) -> ReadBuf;

 public:
  auto len() const -> usize;
  auto capacity() const -> usize;
  auto has_data_left() const -> bool;
  auto buffer() const -> Slice<const u8>;

  void backshift();
  void discard_buffer();

  void consume(usize amount);

  auto fill_buf(Dyn<Read> read) -> Result<Slice<const u8>>;
  auto skip_until(Dyn<Read> read, u8 byte) -> Result<usize>;
  auto read(Dyn<Read> read, Slice<u8> buf) -> Result<usize>;
  auto read_until(Dyn<Read> read, u8 byte, List<u8>& buf) -> Result<usize>;
  auto read_line(Dyn<Read> read, String& buf) -> Result<usize>;
};

class WriteBuf {
  List<u8> _buf{};

 public:
  WriteBuf();
  ~WriteBuf();
  WriteBuf(WriteBuf&&) noexcept;
  WriteBuf& operator=(WriteBuf&&) noexcept;

  static auto with_capacity(usize capacity) -> WriteBuf;

 public:
  auto len() const -> usize;
  auto capacity() const -> usize;
  auto spare_capacity() const -> usize;
  auto buffer() const -> Slice<const u8>;

  auto write(Dyn<Write> write, Slice<const u8> buf) -> Result<usize>;
  auto flush(Dyn<Write> write) -> Result<>;
};

template <class R>
class BufReader : public Read {
  static constexpr usize DEFAULT_BUFF_SIZE = 256U;

  R _inn;
  ReadBuf _buf;

 public:
  explicit BufReader(R inn, ReadBuf buf) noexcept : _inn{mem::move(inn)}, _buf{mem::move(buf)} {}

  ~BufReader() {}

  static auto new_(R inn) -> BufReader {
    return BufReader{mem::move(inn), ReadBuf::with_capacity(DEFAULT_BUFF_SIZE)};
  }

  static auto with_capacity(usize capacity, R inn) -> BufReader {
    return BufReader{mem::move(inn), ReadBuf::with_capacity(capacity)};
  }

 public:
  auto buffer() const -> Slice<const u8> {
    return _buf.buffer();
  }

  auto capacity() const -> usize {
    return _buf.capacity();
  }

  void consume(usize amount) {
    return _buf.consume(amount);
  }

  auto fill_buf() -> Result<Slice<const u8>> {
    return _buf.fill_buf(_inn);
  }

 public:
  auto read(Slice<u8> buf) -> Result<usize> {
    return _buf.read(_inn, buf);
  }

  auto read_until(u8 byte, List<u8>& buf) -> Result<usize> {
    return _buf.read_until(_inn, byte, buf);
  }

  auto skip_until(u8 byte) -> Result<usize> {
    return _buf.skip_until(_inn, byte);
  }

  auto read_line(String& buf) -> Result<usize> {
    return _buf.read_line(_inn, buf);
  }
};

template <class W>
class BufWriter : public Write {
  static constexpr usize DEFAULT_BUF_SIZE = 256;
  W _inn;
  WriteBuf _buf;

 public:
  explicit BufWriter(W inn, WriteBuf buf) noexcept : _inn{mem::move(inn)}, _buf{mem::move(buf)} {}

  ~BufWriter() noexcept {
    (void)this->flush();
  }

  static auto new_(W inn) -> BufWriter {
    return BufWriter{mem::move(inn), WriteBuf::with_capacity(DEFAULT_BUF_SIZE)};
  }

  static auto with_capacity(usize capacity, W inn) -> BufWriter {
    return BufWriter{mem::move(inn), WriteBuf::with_capacity(capacity)};
  }

 public:
  auto inner() -> W& {
    return _inn;
  }

  auto buffer() const -> Slice<const u8> {
    return _buf.buffer();
  }

  auto capacity() const -> usize {
    return _buf.capacity();
  }

  auto spare_capacity() const -> usize {
    return _buf.spare_capacity();
  }

 public:
  auto write(Slice<const u8> buf) -> Result<usize> {
    return _buf.write(_inn, buf);
  }

  auto flush() -> Result<> {
    return _buf.flush(_inn);
  }
};

}  // namespace sfc::io
