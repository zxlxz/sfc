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

struct Read {
  struct Dyn;
  auto read_exact(this Dyn self, Slice<u8> buf) -> Result<>;
  auto read_to_end(this Dyn self, List<u8>& buf) -> Result<usize>;
  auto read_to_string(this Dyn self, String& buf) -> Result<usize>;
};

struct Write {
  struct Dyn;
  auto write_all(this Dyn self, Slice<const u8> buf) -> Result<>;
  auto write_str(this Dyn self, Str buf) -> Result<>;
};

struct Read::Dyn : Read {
 public:
  template <trait::not_<Dyn> X>
  Dyn(X& x) : _impl{&x} {
    _read = [](Dyn& x, auto... u) { return ((X*)(x._impl))->read(u...); };
  }

  auto read(this Dyn& self, Slice<u8> buf) -> Result<usize>;

 private:
  void* _impl;
  decltype(&Dyn::read) _read;
};

struct Write::Dyn : Write {
 public:
  template <trait::not_<Dyn> X>
  Dyn(X& x) : _impl{&x} {
    _write = [](Dyn& x, auto... u) { return ((X*)(x._impl))->write(u...); };
    _flush = [](Dyn& x, auto... u) { return ((X*)(x._impl))->flush(u...); };
  }

  auto write(this Dyn& self, Slice<const u8> buf) -> Result<usize>;
  auto flush(this Dyn& self) -> Result<>;

 private:
  void* _impl;
  decltype(&Dyn::write) _write;
  decltype(&Dyn::flush) _flush;
};

}  // namespace sfc::io
