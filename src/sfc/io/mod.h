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

class DynRead final {
  void* _self;
  Result<usize> (*_read)(void*, Slice<u8> buf);

 public:
  template <trait::not_<DynRead> X>
  DynRead(X& x) : _self{&x}, _read{[](void* p, Slice<u8> buf) { return ((X*)p)->read(buf); }} {}

 public:
  auto read(Slice<u8> buf) -> Result<usize>;
  auto read_exact(Slice<u8> buf) -> Result<>;
  auto read_to_end(List<u8>& buf) -> Result<usize>;
  auto read_to_string(String& buf) -> Result<usize>;
};

class DynWrite final {
  void* _self;
  Result<usize> (*_write)(void*, Slice<const u8> buf);
  Result<> (*_flush)(void*);

 public:
  template <trait::not_<DynWrite> X>
  DynWrite(X& x)
      : _self{&x}
      , _write{[](void* p, Slice<const u8> buf) { return ((X*)p)->write(buf); }}
      , _flush{[](void* p) { return ((X*)p)->flush(); }} {}

 public:
  auto write(Slice<const u8> buf) -> Result<usize>;
  auto write_all(Slice<const u8> buf) -> Result<>;
  auto write_str(Str buf) -> Result<>;
  auto flush() -> Result<>;
};

class Read {
 public:
  auto read_exact(this auto& self, Slice<u8> buf) -> Result<> {
    return DynRead(self).read_exact(buf);
  }

  auto read_to_end(this auto& self, List<u8>& buf) -> Result<usize> {
    return DynRead(self).read_to_end(buf);
  }

  auto read_to_string(this auto& self, String& buf) -> Result<usize> {
    return DynRead(self).read_to_string(buf);
  }
};

class Write {
 public:
  auto write_all(this auto& self, Slice<const u8> buf) -> Result<> {
    return DynWrite(self).write_all(buf);
  }

  auto write_str(this auto& self, Str buf) -> Result<> {
    return DynWrite(self).write_str(buf);
  }
};

}  // namespace sfc::io
