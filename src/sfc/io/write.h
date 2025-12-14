#pragma once

#include "sfc/io/mod.h"
#include "sfc/alloc/vec.h"

namespace sfc::io {

struct Write {
  auto write_all(this auto& self, Slice<const u8> buf) -> Result<> {
    static_assert(requires { self.write(buf); });

    while (!buf.is_empty()) {
      const auto cnt = _TRY(Result{self.write(buf)});
      if (cnt == 0) {
        return Error::WriteZero;
      }
      buf = buf[{cnt, $}];
    }
    return {};
  }

  auto write_char(this auto& self, auto c) -> Result<> {
    const u8 buf[] = {u8(c)};
    _TRY(Result{self.write(buf)});
    return {};
  }

  auto write_str(this auto& self, Str buf) -> Result<> {
    return self.write_all(buf.as_bytes());
  }
};

template <class W>
class BufWriter : Write {
  static constexpr usize BUFF_SIZE = 1024U;

  W _inn;
  Vec<u8> _buf;

 public:
  explicit BufWriter(W&& inn) noexcept : _inn{static_cast<W&&>(inn)}, _buf{Vec<u8>::with_capacity(BUFF_SIZE)} {}

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
      buf = buf[{n, $}];
    }
    _buf.clear();
    return {};
  }
};

}  // namespace sfc::io
