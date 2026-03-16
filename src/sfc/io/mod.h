#pragma once

#include "sfc/alloc.h"

namespace sfc::io {

struct Read {
  auto read_exact(this auto& self, Slice<u8> buf) -> Result<> {
    while (!buf.is_empty()) {
      const auto cnt = _TRY(self.read(buf));
      if (cnt == 0) {
        return Error::UnexpectedEof;
      }
      buf = buf[{cnt, $}];
    }

    return {};
  }

  auto read_to_end(this auto& self, Vec<u8>& buf) -> Result<usize> {
    static constexpr auto PROBE_SIZE = 1024U;

    const auto old_len = buf.len();
    while (true) {
      buf.reserve(PROBE_SIZE);

      auto spare = buf.spare_capacity_mut();
      const auto read_cnt = _TRY(self.read(spare));
      if (read_cnt == 0) {
        break;
      }
      buf.set_len(buf.len() + read_cnt);
    }
    return buf.len() - old_len;
  }

  auto read_to_string(this auto& self, String& buf) -> Result<usize> {
    return self.read_to_end(buf.as_mut_vec());
  }
};

struct Write {
  auto write_all(this auto& self, Slice<const u8> buf) -> Result<> {
    while (!buf.is_empty()) {
      const auto write_cnt = _TRY(self.write(buf));
      if (write_cnt == 0) {
        return Error::WriteZero;
      }
      buf = buf[{write_cnt, $}];
    }
    return {};
  }

  auto write_str(this auto& self, Str buf) -> Result<> {
    return self.write_all(buf.as_bytes());
  }
};

struct SeekFrom {
  enum class Whence {
    Set = 0,
    Cur = 1,
    End = 2,
  };
  i64 offset;
  Whence whence = Whence::Set;

 public:
  static auto Start(u64 offset) -> SeekFrom {
    return SeekFrom{static_cast<i64>(offset), Whence::Set};
  }

  static auto Current(i64 offset) -> SeekFrom {
    return SeekFrom{offset, Whence::Cur};
  }

  static auto End(i64 offset) -> SeekFrom {
    return SeekFrom{offset, Whence::End};
  }
};

}  // namespace sfc::io
