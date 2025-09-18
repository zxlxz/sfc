#pragma once

#include "sfc/io/mod.h"

namespace sfc::io {

static constexpr usize DEFAULT_BUF_SIZE = 4096U;

struct Buffer {
  vec::Buf<u8> _buf = {};
  usize _pos = 0;
  usize _cap = 0;

 public:
  static auto with_capacity(usize capacity) -> Buffer {
    auto res = Buffer{
        ._buf = vec::Buf<u8>::with_capacity(capacity),
        ._pos = 0,
        ._cap = 0,
    };
    return res;
  }

 public:
  auto capacity() const -> usize {
    return _buf.capacity();
  }

  auto is_empty() const -> bool {
    return _pos >= _cap;
  }

  auto buffer() const -> Slice<const u8> {
    return {_buf._ptr + _pos, _cap - _pos};
  }

  auto read_more(auto& r, usize n) -> Result<Slice<const u8>> {
    if (_cap - _pos < n) {
      if (_buf._cap - _pos < n && _pos > 0) {
        ptr::copy(_buf._ptr + _pos, _buf._ptr, _cap - _pos);
        _cap -= _pos;
        _pos = 0;
      }
      const auto amt = _TRY(r.read(_buf.slice_mut(_cap, _buf._cap)));
      _cap += amt;
    }

    const auto cnt = num::min(_cap - _pos, n);
    return Slice<const u8>{_buf._ptr + _pos, cnt};
  }

  auto fill_buf(auto& r) -> Result<Slice<const u8>> {
    if (_pos == _cap) {
      _pos = 0;
      _cap = _TRY(r.read({_buf._ptr, _buf._cap}));
    }
    return Slice<const u8>{_buf._ptr + _pos, _cap - _pos};
  }

  void consume(usize amt) {
    _pos = num::min(_pos + amt, _cap);
  }
};

template <class R>
class BufReader : Read {
  R _inn;
  Buffer _buf;

 public:
  explicit BufReader(R&& inn) noexcept : _inn{mem::move(inn)}, _buf{Buffer::with_capacity(DEFAULT_BUF_SIZE)} {}
  ~BufReader() noexcept = default;

  BufReader(BufReader&&) noexcept = default;
  BufReader& operator=(BufReader&&) noexcept = default;

  BufReader(const BufReader&) = delete;
  BufReader& operator=(const BufReader&) = delete;

 public:
  auto fill_buf() -> Result<Slice<const u8>> {
    return _buf.fill_buf(_inn);
  }

  auto peak(usize n) -> Result<Slice<const u8>> {
    return _buf.read_more(_inn, n);
  }

  void consume(usize n) {
    _buf.consume(n);
  }

  auto read(Slice<u8> buf) -> Result<usize> {
    if (_buf.is_empty() && buf.len() > _buf.capacity()) {
      return _inn.read(buf);
    }

    const auto rem = _TRY(_buf.fill_buf(_inn));
    const auto cnt = rem.read(buf);
    this->consume(cnt);
    return cnt;
  }

  auto read_until(auto&& p, Vec<u8>& buf) -> Result<usize> {
    const auto old_len = buf.len();
    while (true) {
      const auto rem = _TRY(this->fill_buf());
      if (rem.is_empty()) {
        break;
      }

      const auto pos = rem.find(p);
      const auto cnt = pos ? *pos + 1 : rem.len();
      buf.extend_from_slice(rem.slice(0, cnt));
      _buf.consume(cnt);
      if (pos) {
        break;
      }
    }
    return buf.len() - old_len;
  }
};

template <class W>
class BufWriter : Write {
  W _inn;
  Vec<u8> _buf;

 public:
  explicit BufWriter(W&& inn) noexcept : _inn{mem::move(inn)}, _buf{Vec<u8>::with_capacity(DEFAULT_BUF_SIZE)} {}
  ~BufWriter() noexcept = default;

  BufWriter(BufWriter&&) noexcept = default;
  BufWriter& operator=(BufWriter&&) noexcept = default;

  BufWriter(const BufWriter&) = delete;
  BufWriter& operator=(const BufWriter&) = delete;

 public:
  auto buffer() const -> Slice<const u8> {
    return _buf.as_slice();
  }

  auto capacity() const -> usize {
    return _buf.capacity();
  }

  auto spare_capacity() const -> usize {
    return _buf.capacity() - _buf.len();
  }

  auto write(Slice<const u8> buf) -> Result<usize> {
    if (buf.len() < _buf.capacity() - _buf.len()) {
      _buf.extend_from_slice(buf);
      return buf.len();
    }
    _TRY(this->flush());
    if (buf.len() < _buf.capacity()) {
      _buf.extend_from_slice(buf);
      return buf.len();
    }
    return _inn.write(buf);
  }

  auto flush() -> Result<> {
    if (_buf.len() == 0) {
      return {};
    }
    auto buf = _buf.as_slice();
    while (!buf.is_empty()) {
      const auto n = _TRY(_inn.write(buf));
      buf = buf.slice(n);
    }
    return {};
  }
};

}  // namespace sfc::io
