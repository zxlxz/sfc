#if defined(__unix__) || defined(__APPLE__)
#include "sfc/sys/unix/io.inl"
#elif defined(_WIN32)
#include "sfc/sys/windows/io.inl"
#endif

#include "sfc/io/mod.h"

namespace sfc::io {

auto last_os_error() noexcept -> Error {
  const auto os_err = sys::os_error();
  const auto io_err = sys::io_error(os_err);
  return io_err;
}

auto DynRead::read(Slice<u8> buf) -> Result<usize> {
  return _read(_self, buf);
}

auto DynRead::read_exact(Slice<u8> buf) -> Result<> {
  while (!buf.is_empty()) {
    const auto cnt = _TRY(this->read(buf));
    if (cnt == 0) {
      return {io::Error::UnexpectedEof};
    }
    buf = buf[{cnt, $}];
  }

  return Ok{};
}

auto DynRead::read_to_end(List<u8>& buf) -> Result<usize> {
  static constexpr auto PROBE_SIZE = 256U;

  const auto old_len = buf.len();
  while (true) {
    buf.reserve(PROBE_SIZE);

    auto spare = buf.spare_capacity_mut();
    const auto read_cnt = _TRY(this->read(spare));
    if (read_cnt == 0) {
      break;
    }
    buf.set_len(buf.len() + read_cnt);
  }
  return Ok{usize{buf.len() - old_len}};
}

auto DynRead::read_to_string(String& buf) -> Result<usize> {
  return this->read_to_end(buf.as_mut_buf());
}

auto DynWrite::write(Slice<const u8> buf) -> Result<usize> {
  return _write(_self, buf);
}

auto DynWrite::write_all(Slice<const u8> buf) -> Result<> {
  while (!buf.is_empty()) {
    const auto write_cnt = _TRY(this->write(buf));
    if (write_cnt == 0) {
      return Error::WriteZero;
    }
    buf = buf[{write_cnt, $}];
  }
  return Ok{};
}

auto DynWrite::write_str(Str buf) -> Result<> {
  return this->write_all(buf.as_bytes());
}

auto DynWrite::flush() -> Result<> {
  if (!_flush) return Ok{};
  return (_flush)(_self);
}

}  // namespace sfc::io
