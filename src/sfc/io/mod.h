#pragma once

#include "sfc/alloc/list.h"
#include "sfc/alloc/string.h"

namespace sfc::io {

struct SeekFrom {
  enum class Kind { Start = 0, Current = 1, End = 2 };
  Kind _tag{};
  i64 offset;

 public:
  static auto Start(u64 offset) -> SeekFrom {
    return SeekFrom{Kind::Start, i64(offset)};
  }

  static auto Current(i64 offset) -> SeekFrom {
    return SeekFrom{Kind::Current, offset};
  }

  static auto End(i64 offset) -> SeekFrom {
    return SeekFrom{Kind::End, offset};
  }
};

struct Read {
  struct Impl;
  struct VTbl {
    Result<usize> (*read)(void*, Slice<u8>);
  };

  template <class X>
  static constexpr auto vtbl = VTbl{
      .read = [](void* x, Slice<u8> buf) { return ((X*)x)->read(buf); },
  };

 public:
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
    return self.read_to_end(buf.as_mut_list());
  }
};

struct Read::Impl {
  const VTbl* _vtbl;
  void* _impl;

 public:
  auto read(Slice<u8> buf) -> Result<usize> {
    return (_vtbl->read)(_impl, buf);
  }
};

struct Write {
  struct Impl;
  struct VTbl {
    Result<usize> (*write)(void*, Slice<const u8>);
    Result<> (*flush)(void*);
  };

  template <class X>
  static constexpr auto vtbl = VTbl{
      .write = [](void* x, Slice<const u8> buf) { return ((X*)x)->write(buf); },
      .flush = [](void* x) { return ((X*)x)->flush(); },
  };

 public:
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

struct Write::Impl {
  const VTbl* _vtbl;
  void* _impl;

 public:
  auto write(Slice<const u8> buf) -> Result<usize> {
    return (_vtbl->write)(_impl, buf);
  }

  auto flush() -> Result<> {
    return (_vtbl->flush)(_impl);
  }
};

struct Seek {
  struct Impl;
  struct VTbl {
    Result<u64> (*seek)(void*, SeekFrom);
  };

  template <class X>
  static constexpr auto vtbl = VTbl{
      .seek = [](void* x, auto... u) { return ((X*)(x))->seek(u...); },
  };

 public:
  auto seek_relative(this auto& self, i64 offset) -> Result<u64> {
    return self.seek({SeekFrom::Current(offset)});
  }

  auto stream_position(this auto& self) -> Result<u64> {
    return self.seek({SeekFrom::Current(0)});
  }
};

struct Seek::Impl {
  const VTbl* _vtbl;
  void* _impl;

 public:
  auto seek(SeekFrom pos) -> Result<u64> {
    return (_vtbl->seek)(_impl, pos);
  }
};

}  // namespace sfc::io
