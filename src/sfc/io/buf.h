#pragma once

#include "sfc/io/mod.h"

namespace sfc::io {

class Buffer {
  Vec<u8> _buf{};
  usize _pos = 0;

 public:
  static auto with_capacity(usize capacity) -> Buffer {
    auto res = Buffer{};
    res._buf.reserve(capacity);
    return res;
  }

  auto len() const -> usize {
    return _buf.len() - _pos;
  }

  auto capacity() const -> usize {
    return _buf.capacity();
  }

  auto buffer() const -> Slice<const u8> {
    return _buf[{_pos, $}];
  }

  void backshift() {
    if (_pos == 0) {
      return;
    }
    _buf.drain({0, _pos});
    _pos = 0;
  }

  void discard_buffer() {
    _buf.set_len(0);
    _pos = 0;
  }

  void consume(usize amount) {
    _pos = cmp::min(_pos + amount, _buf.len());
  }

  auto read_more(auto& read) -> io::Result<usize> {
    return read.read(_buf.spare_capacity_mut());
  }

  auto fill_buf(auto& read) -> Result<Slice<const u8>> {
    if (_pos == _buf.len()) {
      this->backshift();

      auto ret = this->read_more(read);
      if (ret.is_err()) {
        return ~ret;
      }
    }
    return this->buffer();
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
      const auto fill_res = r.fill_buf();
      if (fill_res.is_err()) {
        return ~fill_res;
      }

      const auto available = *fill_res;
      const auto position = available.iter().position([&](auto c) { return !delim(c); });
      const auto used = position ? *position + 1 : available.len();

      r.consume(used);
      nread += used;
      if (position || used == 0) {
        break;
      }
    }
    return nread;
  }

  auto read_until(this auto& r, u8 delim, Vec<u8>& buf) -> Result<usize> {
    auto nread = 0UL;

    while (true) {
      const auto fill_res = r.fill_buf();
      if (fill_res.is_err()) {
        return ~fill_res;
      }

      const auto available = *fill_res;
      const auto position = available.find(delim);
      const auto used = position ? *position + 1 : available.len();
      buf.extend_from_slice(available[{0, used}]);
      r.consume(used);
      nread += used;

      if (position || used == 0) {
        break;
      }
    }
    return nread;
  }
};

template <class R>
class BufReader : BufRead {
  static constexpr usize DEFAULT_BUFF_SIZE = 1024U;

  R _inn;
  Buffer _buf = Buffer::with_capacity(DEFAULT_BUFF_SIZE);

 public:
  explicit BufReader(R&& inn) noexcept : _inn{static_cast<R&&>(inn)} {}
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

      if (auto ret = _buf.read_more(_inn); ret.is_err()) {
        return ~ret;
      }
    }
    return _buf.buffer()[{0, n}];
  }

  // trait:: io::Read
  auto read(Slice<u8> buf) -> Result<usize> {
    // if we dont't have any buffered data
    // read directly into the user's buffer
    if (_buf.buffer().is_empty() && buf.len() >= _buf.capacity()) {
      _buf.discard_buffer();
      return _inn.read(buf);
    }

    auto fill_res = _buf.fill_buf(_inn);
    if (fill_res.is_err()) {
      return ~fill_res;
    }

    auto rem = *fill_res;
    const auto nread = rem.read(buf).unwrap_or(0);
    _buf.consume(nread);
    return nread;
  }
};

template <class W>
class BufWriter : Write {
  static constexpr usize BUFF_SIZE = 1024U;

  W _inn;
  Vec<u8> _buf{Vec<u8>::with_capacity(BUFF_SIZE)};

 public:
  explicit BufWriter(W&& inn) noexcept : _inn{static_cast<W&&>(inn)} {}

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
  // triat:: io::Read
  auto write(Slice<const u8> buf) -> Result<usize> {
    // write to fuffer
    if (buf.len() < this->spare_capacity()) {
      _buf.extend_from_slice(buf);
      return buf.len();
    }
    // write cold
    if (buf.len() > this->spare_capacity()) {
      if (auto res = this->flush(); res.is_err()) {
        return ~res;
      }
    }
    if (buf.len() >= _buf.capacity()) {
      return _inn.write(buf);
    }
    _buf.extend_from_slice(buf);
    return buf.len();
  }

  // triat:: io::Read
  auto flush() -> Result<> {
    const auto buf = _buf.as_slice();
    auto nwrite = usize{0UL};
    while (nwrite < buf.len()) {
      const auto res = _inn.write(buf[{nwrite, $}]);
      if (res.is_err()) {
        _buf.drain({0, nwrite});
        return ~res;
      }
      nwrite += *res;
    }

    _buf.clear();
    return {};
  }
};

}  // namespace sfc::io
