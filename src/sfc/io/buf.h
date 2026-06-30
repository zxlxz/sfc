#pragma once

#include "sfc/io/mod.h"

namespace sfc::io {

class ReadBuf {
  List<u8> _buf{};
  usize _pos = 0;

 public:
  static auto with_capacity(usize capacity) -> ReadBuf;

  auto len() const -> usize;
  auto capacity() const -> usize;
  auto has_data_left() const -> bool;
  auto buffer() const -> Slice<const u8>;

  void backshift();
  void discard_buffer();

  void consume(usize amount);

  auto fill_buf(DynRead read) -> Result<Slice<const u8>>;
  auto read_more(DynRead read) -> Result<usize>;
  auto skip_until(DynRead read, u8 byte) -> Result<usize>;

  auto peak(DynRead read, usize n) -> Result<Slice<const u8>>;
  auto read(DynRead read, Slice<u8> buf) -> Result<usize>;
  auto read_until(DynRead read, u8 byte, List<u8>& buf) -> Result<usize>;
  auto read_line(DynRead read, String& buf) -> Result<usize>;
};

class WriteBuf {
  List<u8> _buf{};

 public:
  static auto with_capacity(usize capacity) -> WriteBuf;

  auto len() const -> usize;
  auto capacity() const -> usize;
  auto spare_capacity() const -> usize;
  auto buffer() const -> Slice<const u8>;

  auto write(DynWrite write, Slice<const u8> buf) -> Result<usize>;
  auto flush(DynWrite write) -> Result<>;
};

template <class R>
class BufReader : public Read {
  static constexpr usize DEFAULT_BUFF_SIZE = 1024U;
  R _inn;
  ReadBuf _buf = ReadBuf::with_capacity(DEFAULT_BUFF_SIZE);

 public:
  explicit BufReader(R inn) noexcept : _inn{mem::move<R>(inn)} {}
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
  auto peak(usize n) -> Result<Slice<const u8>> {
    return _buf.peak(_inn, n);
  }

  // trait: io::Read
  auto read(Slice<u8> buf) -> Result<usize> {
    return _buf.read(_inn, buf);
  }
};

template <class W>
class BufWriter : public Write {
  static constexpr usize BUFF_SIZE = 1024U;

  W _inn;
  WriteBuf _buf = WriteBuf::with_capacity(BUFF_SIZE);

 public:
  explicit BufWriter(W inn) noexcept : _inn{mem::move<W>(inn)} {}

  ~BufWriter() noexcept {
    (void)this->flush();
  }

  BufWriter(BufWriter&&) noexcept = default;
  BufWriter& operator=(BufWriter&&) noexcept = default;

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
  // trait: io::Read
  auto write(Slice<const u8> buf) -> Result<usize> {
    return _buf.write(_inn, buf);
  }

  // trait: io::Read
  auto flush() -> Result<> {
    return _buf.flush(_inn);
  }
};

}  // namespace sfc::io
