#include "sfc/io/mod.h"
#include "sfc/sys/io.h"

namespace sfc::io {

auto SeekFrom::Start(u64 offset) -> SeekFrom {
  const auto off = num::saturating_cast<i64>(offset);
  return SeekFrom{Kind::Start, off};
}

auto SeekFrom::Current(i64 offset) -> SeekFrom {
  return SeekFrom{Kind::Current, offset};
}

auto SeekFrom::End(i64 offset) -> SeekFrom {
  return SeekFrom{Kind::End, offset};
}

auto last_os_error() noexcept -> Error {
  const auto os_err = sys::os_error();
  const auto io_err = sys::io_error(os_err);
  return io_err;
}

auto Read::Dyn::read(this Dyn& self, Slice<u8> buf) -> Result<usize> {
  return self._read(self, buf);
}

auto Read::read_exact(this Dyn self, Slice<u8> buf) -> Result<> {
  while (!buf.is_empty()) {
    const auto cnt = _TRY(self.read(buf));
    if (cnt == 0) {
      return {io::Error::UnexpectedEof};
    }
    buf = buf[{cnt, $}];
  }

  return Ok{};
}

auto Read::read_to_end(this Dyn self, List<u8>& buf) -> Result<usize> {
  static constexpr auto PROBE_SIZE = 256U;

  const auto old_len = buf.len();
  while (true) {
    buf.reserve(PROBE_SIZE);

    auto read_buf = buf.spare_capacity_mut();
    const auto read_cnt = _TRY(self.read(read_buf));
    if (read_cnt == 0) {
      break;
    }
    buf.set_len(buf.len() + read_cnt);
  }
  return Ok{usize{buf.len() - old_len}};
}

auto Read::read_to_string(this Dyn self, String& buf) -> Result<usize> {
  return self.read_to_end(buf.as_mut_buf());
}

auto Write::Dyn::write(this Dyn& self, Slice<const u8> buf) -> Result<usize> {
  return self._write(self, buf);
}

auto Write::Dyn::flush(this Dyn& self) -> Result<> {
  return self._flush(self);
}

auto Write::write_all(this Dyn self, Slice<const u8> buf) -> Result<> {
  while (!buf.is_empty()) {
    const auto write_cnt = _TRY(self.write(buf));
    if (write_cnt == 0) {
      return Error::WriteZero;
    }
    buf = buf[{write_cnt, $}];
  }
  return Ok{};
}

auto Write::write_str(this Dyn self, Str buf) -> Result<> {
  return self.write_all(buf.as_bytes());
}

}  // namespace sfc::io
