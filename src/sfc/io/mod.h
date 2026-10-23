#pragma once

#include "sfc/alloc/list.h"
#include "sfc/alloc/string.h"

namespace sfc::io {

struct SeekFrom {
  enum class Kind { Start = 0, Current = 1, End = 2 };
  Kind _tag{};
  i64 offset;

 public:
  static auto Start(u64 offset) -> SeekFrom;
  static auto Current(i64 offset) -> SeekFrom;
  static auto End(i64 offset) -> SeekFrom;
};

class Read {
  friend Dyn<Read>;
  struct VTbl {
    Result<usize> (*_read)(void*, Slice<u8>);
  };

  template <class X>
  static constexpr auto vtbl = VTbl{
      ._read = [](void* x, auto... u) { return ((X*)(x))->read(u...); },
  };

 public:
  auto read(this auto& self, Slice<u8> buf) -> Result<usize> {
    return (self._vtbl->_read)(self._impl, buf);
  }

  auto read_exact(this auto& self, Slice<u8> buf) -> Result<> {
    while (!buf.is_empty()) {
      const auto cnt = _TRY(self.read(buf));
      if (cnt == 0) {
        return {io::Error::UnexpectedEof};
      }
      buf = buf[{cnt, $}];
    }

    return Ok{};
  }

  auto read_to_end(this auto& self, List<u8>& buf) -> Result<usize> {
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

  auto read_to_string(this auto& self, String& buf) -> Result<usize> {
    return self.read_to_end(buf.as_mut_buf());
  }
};

class Write {
  friend Dyn<Write>;
  struct VTbl {
    Result<usize> (*write)(void*, Slice<const u8>);
    Result<> (*flush)(void*);
  };

  template <class X>
  static constexpr auto vtbl = VTbl{
      .write = [](void* x, auto... u) { return ((X*)(x))->write(u...); },
      .flush = [](void* x, auto... u) { return ((X*)(x))->flush(u...); },
  };

 public:
  auto write(this auto& self, Slice<const u8> buf) -> Result<usize> {
    return (self._vtbl->write)(self._impl, buf);
  }

  auto flush(this auto& self) -> Result<> {
    return (self._vtbl->flush)(self._impl);
  }

  auto write_all(this auto& self, Slice<const u8> buf) -> Result<> {
    while (!buf.is_empty()) {
      const auto cnt = _TRY(self.write(buf));
      if (cnt == 0) {
        return {io::Error::WriteZero};
      }
      buf = buf[{cnt, $}];
    }

    return Ok{};
  }

  auto write_str(this auto& self, Str buf) -> Result<> {
    return self.write_all(buf.as_bytes());
  }
};

template <trait::tv_copy_ T>
struct FromBytes {
  static auto read_from(auto& read) -> Result<T> {
    u8 buf[sizeof(T)];
    _TRY(read.read_exact(buf));
    return __builtin_bit_cast(T, buf);
  }
};

}  // namespace sfc::io
