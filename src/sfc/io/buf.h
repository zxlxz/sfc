#pragma once

#include "sfc/io/mod.h"

namespace sfc::io {

class ReadBuf {
  List<u8> _buf{};
  usize _pos = 0;

 public:
  static auto with_capacity(usize capacity) -> ReadBuf;

 public:
  auto len() const -> usize;
  auto capacity() const -> usize;
  auto has_data_left() const -> bool;
  auto buffer() const -> Slice<const u8>;

  void backshift();
  void discard_buffer();

  void consume(usize amount);

  auto fill_buf(DynRead read) -> Result<Slice<const u8>>;
  auto skip_until(DynRead read, u8 byte) -> Result<usize>;

  auto read(DynRead read, Slice<u8> buf) -> Result<usize>;
  auto read_until(DynRead read, u8 byte, List<u8>& buf) -> Result<usize>;
  auto read_line(DynRead read, String& buf) -> Result<usize>;
};

class WriteBuf {
  List<u8> _buf{};

 public:
  static auto with_capacity(usize capacity) -> WriteBuf;

 public:
  auto len() const -> usize;
  auto capacity() const -> usize;
  auto spare_capacity() const -> usize;
  auto buffer() const -> Slice<const u8>;

  auto write(DynWrite write, Slice<const u8> buf) -> Result<usize>;
  auto flush(DynWrite write) -> Result<>;
};

template <class R>
class BufReader : public Read {
  static constexpr usize DEFAULT_BUFF_SIZE = 256U;
  R _inn;
  ReadBuf _buf;

 public:
  explicit BufReader(R inn) noexcept : _inn{mem::move(inn)}, _buf{ReadBuf::with_capacity(DEFAULT_BUFF_SIZE)} {}

  explicit BufReader(R inn, ReadBuf buf) noexcept : _inn{mem::move(inn)}, _buf{mem::move(buf)} {}

  ~BufReader() noexcept = default;

  BufReader(BufReader&&) noexcept = default;
  BufReader& operator=(BufReader&&) noexcept = default;

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
    return _buf.read(DynRead::of(_inn), buf);
  }

  auto read_until(u8 byte, List<u8>& buf) -> Result<usize> {
    return _buf.read_until(DynRead::of(_inn), byte, buf);
  }

  auto skip_until(u8 byte) -> Result<usize> {
    return _buf.skip_until(DynRead::of(_inn), byte);
  }

  auto read_line(String& buf) -> Result<usize> {
    return _buf.read_line(DynRead::of(_inn), buf);
  }
};

template <class W>
class BufWriter : public Write {
  static constexpr usize DEFAULT_BUF_SIZE = 256;
  W _inn;
  WriteBuf _buf;

 public:
  explicit BufWriter(W inn) noexcept : _inn{mem::move(inn)}, _buf{WriteBuf::with_capacity(DEFAULT_BUF_SIZE)} {}

  explicit BufWriter(W inn, WriteBuf buf) noexcept : _inn{mem::move(inn)}, _buf{mem::move(buf)} {}

  ~BufWriter() noexcept {
    (void)this->flush();
  }

  BufWriter(BufWriter&&) noexcept = default;
  BufWriter& operator=(BufWriter&&) noexcept = default;

  static auto with_capacity(usize capacity, W inn) -> BufWriter {
    auto buf = WriteBuf::with_capacity(capacity);
    return BufWriter{inn, mem::move(buf)};
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
    return _buf.write(DynWrite::of(_inn), buf);
  }

  auto flush() -> Result<> {
    return _buf.flush(DynWrite::of(_inn));
  }
};

}  // namespace sfc::io
