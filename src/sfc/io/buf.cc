#include "sfc/io/buf.h"

namespace sfc::io {

auto Buffer::with_capacity(usize capacity) -> Buffer {
  auto res = Buffer{};
  res._buf.reserve(capacity);
  return res;
}

auto Buffer::len() const -> usize {
  return _buf.len() - _pos;
}

auto Buffer::capacity() const -> usize {
  return _buf.capacity();
}

auto Buffer::buffer() const -> Slice<const u8> {
  return _buf[{_pos, _buf.len()}];
}

void Buffer::backshift() {
  if (_pos == 0) {
    return;
  }
  _buf.drain({0, _pos});
  _pos = 0;
}

void Buffer::discard_buffer() {
  _buf.set_len(0);
  _pos = 0;
}

void Buffer::consume(usize amount) {
  _pos = cmp::min(_pos + amount, _buf.len());
}

}  // namespace sfc::io
