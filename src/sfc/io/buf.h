#pragma once

#include "sfc/io/mod.h"

namespace sfc::io {

static constexpr usize DEFAULT_BUF_SIZE = 4096U;

template <class R>
class BufReader : Read {
  R _inn;
  Vec<u8> _buf = {};
  usize _pos = 0;

 public:
  explicit BufReader(R&& inn) noexcept : _inn{static_cast<R&&>(inn)}, _buf{Vec<u8>::with_capacity(DEFAULT_BUF_SIZE)} {}
  ~BufReader() noexcept = default;

  BufReader(BufReader&&) noexcept = default;
  BufReader& operator=(BufReader&&) noexcept = default;

  BufReader(const BufReader&) = delete;
  BufReader& operator=(const BufReader&) = delete;

 public:
  auto fill_buf() -> Result<Slice<const u8>> {
    if (_pos == _buf.len()) {
      const auto cnt = _TRY(Result{_inn.read(_buf.spare_capacity_mut())});
      _pos = 0;
      _buf.set_len(cnt);
    }
    return _buf.as_slice().slice(_pos);
  }

  void consume(usize amt) {
    _pos = num::min(_pos + amt, _buf.len());
  }

  auto peak(usize n) -> Result<Slice<const u8>> {
    if (_buf.len() - _pos < n) {
      _buf.drain(0, _pos);
      _pos = 0;

      const auto cnt = _TRY(Result{_inn.read(_buf.spare_capacity_mut())});
      _buf.set_len(_buf.len() + cnt);
    }
    return _buf.slice(_pos, _pos + n);
  }

  auto read(Slice<u8> buf) -> Result<usize> {
    if (_buf.is_empty() && buf.len() > _buf.capacity()) {
      return _inn.read(buf);
    }

    const auto rem = this->fill_buf();
    const auto cnt = rem.read(buf);
    this->consume(cnt);
    return cnt;
  }

  auto skip(auto&& p) -> Result<usize> {
    auto res = usize{0UL};
    while (true) {
      const auto rem = _TRY(this->fill_buf());
      const auto pos = rem.iter().position([&](auto c) { return !p(c); });
      const auto cnt = pos.unwrap_or(rem.len());
      this->consume(cnt);
      res += cnt;
      if (pos) {
        break;
      }
    }
    return res;
  }

  auto read_until(auto&& p, Vec<u8>& buf) -> Result<usize> {
    const auto old_len = buf.len();
    while (true) {
      const auto rem = _TRY(this->fill_buf());
      const auto pos = rem.iter().position(p);
      const auto cnt = pos.unwrap_or(rem.len() - 1) + 1;
      buf.extend_from_slice(rem.slice(0, cnt));
      this->consume(cnt);
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
    _buf.clear();
    return {};
  }
};

}  // namespace sfc::io
