#include "sfc/io/mod.h"
#include "sfc/sys/io.h"

namespace sfc::io {

auto Read::read(Slice<u8> buf) -> Result<usize> {
  return _vtbl.read(_self, buf);
}

auto Read::read_exact(Slice<u8> buf) -> Result<> {
  while (!buf.is_empty()) {
    const auto cnt = _TRY(read(buf));
    if (cnt == 0) {
      return {io::Error::UnexpectedEof};
    }
    buf = buf[{cnt, $}];
  }

  return Ok{};
}

auto Read::read_to_end(List<u8>& buf) -> Result<usize> {
  static constexpr auto PROBE_SIZE = 256U;

  const auto old_len = buf.len();
  while (true) {
    buf.reserve(PROBE_SIZE);

    auto read_buf = buf.spare_capacity_mut();
    const auto read_cnt = _TRY(read(read_buf));
    if (read_cnt == 0) {
      break;
    }
    buf.set_len(buf.len() + read_cnt);
  }
  return Ok{usize{buf.len() - old_len}};
}

auto Read::read_to_string(String& buf) -> Result<usize> {
  return this->read_to_end(buf.as_mut_list());
}

auto Write::write(Slice<const u8> buf) -> Result<usize> {
  return (_vtbl.write)(_self, buf);
}

auto Write::flush() -> Result<> {
  return (_vtbl.flush)(_self);
}

auto Write::write_all(Slice<const u8> buf) -> Result<> {
  while (!buf.is_empty()) {
    const auto cnt = _TRY(this->write(buf));
    if (cnt == 0) {
      return {io::Error::WriteZero};
    }
    buf = buf[{cnt, $}];
  }

  return Ok{};
}

auto Write::write_str(Str buf) -> Result<> {
  return this->write_all(buf.as_bytes());
}

auto Seek::seek(SeekFrom pos) -> Result<u64> {
  return (_vtbl.seek)(_self, pos);
}

auto Seek::seek_relative(i64 offset) -> Result<u64> {
  return this->seek({SeekFrom::Current(offset)});
}

auto Seek::stream_position() -> Result<u64> {
  return this->seek({SeekFrom::Current(0)});
}

auto last_os_error() noexcept -> Error {
  const auto os_err = sys::os_error();
  const auto io_err = sys::io_error(os_err);
  return io_err;
}

}  // namespace sfc::io
