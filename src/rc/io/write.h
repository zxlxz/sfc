#pragma once

#include "rc/io/mod.h"

namespace rc::io {

template <class Self>
struct Write {
  Self& _self;

  template <class T>
  auto write(Slice<T> buf) -> usize {
    const auto ret = _self.write(buf);
    return ret;
  }

  template <class T>
  auto write_all(Slice<T> buf) -> void {
    while (!buf.is_empty()) {
      auto cnt = _self.write(buf);
      if (cnt == 0) {
        rc::panic(u8"rc::io::Write: write failed");
      }
      buf._ptr += cnt;
      buf._len -= cnt;
    }
  }
};

template <class Write>
struct BufWriter {
  Write _out;
  Vec<u8> _buf;

  explicit BufWriter(Write&& out) noexcept
      : _out{rc::move(out)}, _buf{Vec<u8>::with_capacity(8192)} {}

  explicit BufWriter(Write&& out, Vec<u8>&& buf) noexcept
      : _out{rc::move(out)}, _buf{rc::move(buf)} {}

  ~BufWriter() { this->flush(); }

  BufWriter(BufWriter&&) noexcept = default;

  static auto with_capacity(usize capacity, Write inner) -> BufWriter {
    auto buf = Vec<u8>::with_capacity(capacity);
    return BufWriter{rc::move(inner), rc::move(buf)};
  }

  auto write(Slice<const u8> buf) -> usize {
    if (_buf.len() + buf.len() > _buf.capacity()) {
      this->flush();
    }
    if (buf.len() >= _buf.capacity()) {
      _out.write(buf);
    } else {
      _buf.append_elements(buf);
    }
    return buf.len();
  }

  auto flush() -> void {
    if (_buf.is_empty()) return;
    (void)_out.write(_buf.as_slice());
    _buf.clear();
  }
};

template <class T>
BufWriter(T &&)->BufWriter<T>;

}  // namespace rc::io
