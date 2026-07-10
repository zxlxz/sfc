#pragma once

#include "sfc/alloc.h"

namespace sfc::io {

class DynRead {
  using read_t = Result<usize> (*)(void*, Slice<u8> buf);
  void* _self;
  read_t _read;

 public:
  template <class X>
  DynRead(X& impl) : _self{&impl}, _read{ops::dyn_fn<&X::read>()} {}

 public:
  auto read(Slice<u8> buf) -> Result<usize>;
  auto read_exact(Slice<u8> buf) -> Result<>;
  auto read_to_end(List<u8>& buf) -> Result<usize>;
  auto read_to_string(String& buf) -> Result<usize>;
};

class DynWrite {
  using write_t = Result<usize> (*)(void*, Slice<const u8> buf);
  using flush_t = Result<> (*)(void*);
  void* _self;
  write_t _write;
  flush_t _flush{nullptr};

 public:
  template <class X>
  DynWrite(X& impl) : _self{&impl}, _write{ops::dyn_fn<&X::write>()} {
    if constexpr (requires{ impl.flush(); }) {
      _flush = ops::dyn_fn<&X::flush>();
    }
  }

 public:
  auto write(Slice<const u8> buf) -> Result<usize>;
  auto write_all(Slice<const u8> buf) -> Result<>;
  auto write_str(Str buf) -> Result<>;
  auto flush() -> Result<>;
};

struct Read {
  auto read_exact(this auto& self, Slice<u8> buf) -> Result<> {
    return DynRead{self}.read_exact(buf);
  }

  auto read_to_end(this auto& self, List<u8>& buf) -> Result<usize> {
    return DynRead{self}.read_to_end(buf);
  }

  auto read_to_string(this auto& self, String& buf) -> Result<usize> {
    return DynRead{self}.read_to_string(buf);
  }
};

struct Write {
  auto write_all(this auto& self, Slice<const u8> buf) -> Result<> {
    return DynWrite{self}.write_all(buf);
  }

  auto write_str(this auto& self, Str buf) -> Result<> {
    return DynWrite{self}.write_str(buf);
  }
};

struct SeekFrom {
  u8 _tag = 0;
  i64 offset;

 public:
  static auto Start(u64 offset) -> SeekFrom {
    return SeekFrom{0, num::cast_signed(offset)};
  }

  static auto Current(i64 offset) -> SeekFrom {
    return SeekFrom{1, offset};
  }

  static auto End(i64 offset) -> SeekFrom {
    return SeekFrom{2, offset};
  }
};

}  // namespace sfc::io
