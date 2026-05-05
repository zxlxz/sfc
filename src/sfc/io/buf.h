#pragma once

#include "sfc/io/mod.h"

namespace sfc::io {

class Buffer {
  List<u8> _buf{};
  usize _pos = 0;

 public:
  static auto with_capacity(usize capacity) -> Buffer;

  auto len() const -> usize;
  auto capacity() const -> usize;
  auto buffer() const -> Slice<const u8>;

  void backshift();
  void discard_buffer();
  void consume(usize amount);

  auto read_more(auto& read) -> io::Result<usize> {
    auto buf = _buf.spare_capacity_mut();
    const auto nread = _TRY(read.read(buf));
    _buf.set_len(_buf.len() + nread);
    return Ok{nread};
  }

  auto fill_buf(auto& read) -> Result<Slice<const u8>> {
    if (_pos == _buf.len()) {
      this->backshift();
      _TRY(this->read_more(read));
    }
    return Ok{this->buffer()};
  }
};

struct BufRead : Read {
 public:
  auto fill_buf() -> Result<Slice<const u8>> = delete;
  auto consume(usize amount) = delete;

 public:
  auto skip(this auto& r, auto&& delim) -> Result<usize> {
    auto nread = usize{0UL};

    while (true) {
      const auto available = _TRY(r.fill_buf());
      const auto position = available.iter().position([&](auto c) { return !delim(c); });
      const auto used = position ? *position + 1 : available.len();

      r.consume(used);
      nread += used;
      if (position || used == 0) {
        break;
      }
    }
    return Ok{nread};
  }

  auto read_until(this auto& r, u8 delim, List<u8>& buf) -> Result<usize> {
    auto nread = 0UL;

    while (true) {
      const auto available = _TRY(r.fill_buf());
      const auto position = available.find(delim);
      const auto used = position ? *position + 1 : available.len();
      buf.extend_from_slice(available[{0, used}]);
      r.consume(used);
      nread += used;

      if (position || used == 0) {
        break;
      }
    }
    return Ok{nread};
  }
};

template <class R>
class BufReader : public BufRead {
  static constexpr usize DEFAULT_BUFF_SIZE = 1024U;

  R _inn;
  Buffer _buf = Buffer::with_capacity(DEFAULT_BUFF_SIZE);

 public:
  explicit BufReader(R inn) noexcept : _inn{static_cast<R&&>(inn)} {}
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
    if (n > _buf.len()) {
      _buf.backshift();
      _TRY(_buf.fill_buf(_inn));
    }
    const auto buf = _buf.buffer();
    return Ok{buf[{0, n}]};
  }

  // trait: io::Read
  auto read(Slice<u8> buf) -> Result<usize> {
    // if we dont't have any buffered data
    // read directly into the user's buffer
    if (_buf.buffer().is_empty() && buf.len() >= _buf.capacity()) {
      _buf.discard_buffer();
      return _inn.read(buf);
    }

    auto rem = _TRY(_buf.fill_buf(_inn));
    const auto nread = rem.read(buf).unwrap_or(0);
    _buf.consume(nread);
    return Ok{nread};
  }
};

template <class W>
class BufWriter : public Write {
  static constexpr usize BUFF_SIZE = 1024U;

  W _inn;
  List<u8> _buf{List<u8>::with_capacity(BUFF_SIZE)};

 public:
  explicit BufWriter(W inn) noexcept : _inn{static_cast<W&&>(inn)} {}

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
    return _buf.as_slice();
  }

  auto capacity() const -> usize {
    return _buf.capacity();
  }

  auto spare_capacity() const -> usize {
    return _buf.capacity() - _buf.len();
  }

 public:
  // trait: io::Read
  auto write(Slice<const u8> buf) -> Result<usize> {
    const auto buf_len = buf.len();
    if (buf_len > this->spare_capacity()) {
      _TRY(this->flush());
    }

    // _buf is empty or buf is small enough to fit into _buf
    // just write cold to _buf
    if (buf_len <= _buf.capacity()) {
      _buf.extend_from_slice(buf);
    } else {
      _TRY(_inn.write(buf));
    }
    return Ok{buf_len};
  }

  // triat:: io::Read
  auto flush() -> Result<> {
    while (!_buf.is_empty()) {
      const auto nwrite = _TRY(_inn.write(_buf.as_slice()));
      _buf.drain({0, nwrite});
    }
    _buf.clear();
    return Ok{};
  }
};

}  // namespace sfc::io
