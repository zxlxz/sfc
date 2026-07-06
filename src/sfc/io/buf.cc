#include "sfc/io/buf.h"

namespace sfc::io {

auto ReadBuf::with_capacity(usize capacity) -> ReadBuf {
  auto res = ReadBuf{};
  res._buf.reserve(capacity);
  return res;
}

auto ReadBuf::len() const -> usize {
  return _buf.len() - _pos;
}

auto ReadBuf::capacity() const -> usize {
  return _buf.capacity();
}

auto ReadBuf::has_data_left() const -> bool {
  return _buf.len() > _pos;
}

auto ReadBuf::buffer() const -> Slice<const u8> {
  return _buf[{_pos, _buf.len()}];
}

void ReadBuf::backshift() {
  if (_pos == 0) {
    return;
  }
  _buf.drain({0, _pos});
  _pos = 0;
}

void ReadBuf::discard_buffer() {
  _buf.set_len(0);
  _pos = 0;
}

void ReadBuf::consume(usize amount) {
  _pos = cmp::min(_pos + amount, _buf.len());
}

auto ReadBuf::read_more(DynRead read) -> Result<usize> {
  auto buf = _buf.spare_capacity_mut();
  const auto nread = _TRY(read.read(buf));
  _buf.set_len(_buf.len() + nread);
  return {nread};
}

auto ReadBuf::fill_buf(DynRead read) -> Result<Slice<const u8>> {
  if (_pos == _buf.len()) {
    this->backshift();
    _TRY(this->read_more(read));
  }
  return {this->buffer()};
}

auto ReadBuf::skip_until(DynRead read, u8 byte) -> Result<usize> {
  auto nread = usize{0};
  while (true) {
    const auto available = _TRY(this->fill_buf(read));
    const auto position = available.find(byte);
    const auto used = position ? *position + 1 : available.len();
    this->consume(used);
    nread += used;

    if (position || used == 0) {
      break;
    }
  }
  return {nread};
}

auto ReadBuf::peak(DynRead read, usize n) -> Result<Slice<const u8>> {
  if (n > _buf.len()) {
    this->backshift();
    _TRY(this->fill_buf(read));
  }

  const auto buf = _buf[{0, n}];
  return Slice<const u8>{buf};
}

auto ReadBuf::read(DynRead read, Slice<u8> buf) -> Result<usize> {
  if (!this->has_data_left() && buf.len() >= _buf.capacity()) {
    this->discard_buffer();
    return read.read(buf);
  }

  auto rem = _TRY(this->fill_buf(read));
  const auto nread = rem.read(buf).unwrap_or(0);
  this->consume(nread);
  return {nread};
}

auto ReadBuf::read_until(DynRead read, u8 byte, List<u8>& buf) -> Result<usize> {
  auto nread = usize{0};
  while (true) {
    const auto available = _TRY(this->fill_buf(read));
    const auto position = available.find(byte);
    const auto used = position ? *position + 1 : available.len();
    buf.extend_from_slice(available[{0, used}]);
    this->consume(used);
    nread += used;

    if (position || used == 0) {
      break;
    }
  }
  return {nread};
}

auto ReadBuf::read_line(DynRead read, String& buf) -> Result<usize> {
  return this->read_until(read, '\n', buf.as_mut_buf());
}

auto WriteBuf::with_capacity(usize capacity) -> WriteBuf {
  auto res = WriteBuf{};
  res._buf.reserve(capacity);
  return res;
}

auto WriteBuf::len() const -> usize {
  return _buf.len();
}

auto WriteBuf::capacity() const -> usize {
  return _buf.capacity();
}

auto WriteBuf::spare_capacity() const -> usize {
  return _buf.capacity() - _buf.len();
}

auto WriteBuf::buffer() const -> Slice<const u8> {
  return _buf.as_slice();
}

auto WriteBuf::write(DynWrite write, Slice<const u8> buf) -> Result<usize> {
  const auto buf_len = buf.len();
  if (buf_len > this->spare_capacity()) {
    _TRY(this->flush(write));
  }

  // _buf is empty or buf is small enough to fit into _buf
  // just write cold to _buf
  if (buf_len <= _buf.capacity()) {
    _buf.extend_from_slice(buf);
  } else {
    _TRY(write.write(buf));
  }
  return {buf_len};
}

auto WriteBuf::flush(DynWrite write) -> Result<> {
  while (!_buf.is_empty()) {
    const auto nwrite = _TRY(write.write(_buf.as_slice()));
    _buf.drain({0, nwrite});
  }
  _buf.clear();
  return Tuple{};
}

}  // namespace sfc::io
