#pragma once

#include "mod.h"

namespace sfc::io {

template <class W>
class BufWriter {
  W _inn;
  Vec<u8> _buf;

 public:
  static auto from(W inn) -> BufWriter;

  static auto with_capacity(usize capacity, W inn) -> BufWriter;

  auto buffer() const -> Slice<u8>;

  auto capacity() const -> usize;

  auto get_mut() -> W&;

  auto get_ref() const -> const W&;

 public:
  auto flush() -> Result<> {
    if (_buf.is_empty()) {
      return _;
    }
    auto res = _inn.write_all(_buf.as_slice());
    _buf.clear();
    return _;
  }

  auto write(Slice<const u8> buf) -> Result<usize> {
    if (buf.len() + _buf.len() >= _buf.capacity()) {
      this->flush();
    }
    if (buf.len() >= _buf.capacity()) {
      return _inn.write(buf);
    }
    _buf.extend_from_slice(buf);
    return buf.len();
  }

  auto write_all(Slice<const u8> buf) -> Result<usize> {
    if (buf.len() + _buf.len() >= _buf.capacity()) {
      this->flush();
    }
    if (buf.len() >= _buf.capacity()) {
      return _inn.write_all(buf);
    }
    _buf.extend_from_slice(buf);
    return buf.len();
  }

  auto write_str(Str s) -> Result<usize> {
    return this->write_all(s.as_bytes());
  }

  auto write_fmt(const auto& fmts, const auto&... args) -> Result<> {
    auto imp = fmt::Fmter<BufWriter>{*this};
    imp.write_fmt(fmts, args...);
    return _;
  }
};

}  // namespace sfc::io
