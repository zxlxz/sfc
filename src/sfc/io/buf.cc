#include "sfc/io/buf.h"

namespace sfc::io {

namespace detail {
inline auto memchr(Slice<const u8> buf, u8 byte) -> Option<usize> {
  const auto ptr = (const u8*)__builtin_memchr(buf._ptr, byte, buf._len);
  if (ptr == nullptr) {
    return {};
  }
  return usize(ptr - buf._ptr);
}
}  // namespace detail

auto BufReader::new_(Read inn, Slice<u8> buf) -> BufReader {
  return BufReader{._inn = inn, ._buf = buf, ._start = 0, ._end = 0};
}

auto BufReader::capacity() const -> usize {
  return _buf._len;
}

auto BufReader::has_data_left() const -> bool {
  return _end > _start;
}

auto BufReader::buffer() const -> Slice<const u8> {
  return _buf[{_start, _end}];
}

void BufReader::backshift() {
  if (_start == 0) {
    return;
  }
  ptr::copy(_buf._ptr + _start, _buf._ptr, _end - _start);
  _end -= _start;
  _start = 0;
}

void BufReader::discard_buffer() {
  _start = 0;
  _end = 0;
}

void BufReader::consume(usize amount) {
  if (amount > _end - _start) {
    amount = _end - _start;
  }
  _start += amount;
}

auto BufReader::fill_buf() -> Result<Slice<const u8>> {
  if (!this->has_data_left()) {
    const auto nread = _TRY(_inn.read(_buf));
    _start = 0;
    _end = nread;
  }
  return {this->buffer()};
}

auto BufReader::skip_until(u8 byte) -> Result<usize> {
  auto nread = usize{0};
  while (true) {
    const auto available = _TRY(this->fill_buf());
    const auto position = detail::memchr(available, byte);
    const auto used = position ? *position + 1 : available.len();
    this->consume(used);
    nread += used;

    if (position || used == 0) {
      break;
    }
  }
  return {nread};
}

auto BufReader::read(Slice<u8> buf) -> Result<usize> {
  if (!this->has_data_left() && buf.len() >= this->capacity()) {
    this->discard_buffer();
    return _inn.read(buf);
  }

  auto rem = _TRY(this->fill_buf());
  const auto nread = rem.read(buf).unwrap_or(0);
  this->consume(nread);
  return {nread};
}

auto BufReader::read_until(u8 byte, List<u8>& buf) -> Result<usize> {
  auto nread = usize{0};
  while (true) {
    const auto available = _TRY(this->fill_buf());
    const auto position = detail::memchr(available, byte);
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

auto BufReader::read_line(String& buf) -> Result<usize> {
  return this->read_until('\n', buf.as_mut_list());
}

auto BufWriter::new_(Write inn, Slice<u8> buf) -> BufWriter {
  return BufWriter{._inn = inn, ._buf = buf, ._end = 0};
}

auto BufWriter::capacity() const -> usize {
  return _buf.len();
}

auto BufWriter::buffer() const -> Slice<const u8> {
  return _buf[{0, _end}];
}

auto BufWriter::spare_capacity_mut() -> Slice<u8> {
  return _buf[{_end, $}];
}

auto BufWriter::write(Slice<const u8> buf) -> Result<usize> {
  const auto buf_len = buf.len();

  if (buf_len > _buf.len() - _end) {
    if (_end != 0) {
      _TRY(this->flush());
    }

    if (buf_len > _buf.len()) {
      return _inn.write(buf);
    }
  }

  const auto dst_ptr = _buf._ptr + _end;
  ptr::copy_nonoverlapping(buf._ptr, dst_ptr, buf_len);
  _end += buf_len;
  return buf_len;
}

auto BufWriter::flush() -> Result<> {
  struct Adaptor {
    BufWriter& _self;
    Slice<const u8> _remain = {};

   public:
    ~Adaptor() {
      ptr::copy(_remain._ptr, _self._buf._ptr, _remain.len());
      _self._end = _remain.len();
    }

   public:
    auto flush() -> Result<> {
      while (!_remain.is_empty()) {
        const auto nwrite = _TRY(_self._inn.write(_remain));
        if (nwrite == 0) {
          return io::Error::UnexpectedEof;
        }
        _remain = _remain[{nwrite, $}];
      }
      return Ok{};
    }
  };

  auto adaptor = Adaptor{*this, _buf[{0, _end}]};
  _TRY(adaptor.flush());
  return _inn.flush();
}

}  // namespace sfc::io
