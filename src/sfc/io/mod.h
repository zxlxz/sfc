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
  struct VTbl {
    Result<usize> (*read)(void*, Slice<u8>);
  };

  template <class X>
  static auto vtbl() -> const VTbl& {
    static const VTbl vtbl = {
        .read = [](void* x, Slice<u8> buf) { return ((X*)(x))->read(buf); },
    };
    return vtbl;
  }

 public:
  struct Dyn;
  auto read_exact(this Dyn self, Slice<u8> buf) -> Result<>;
  auto read_to_end(this Dyn self, List<u8>& buf) -> Result<usize>;
  auto read_to_string(this Dyn self, String& buf) -> Result<usize>;
};

class Write {
  struct VTbl {
    Result<usize> (*write)(void*, Slice<const u8>);
    Result<> (*flush)(void*);
  };

  template <class X>
  static auto vtbl() -> const VTbl& {
    static const VTbl vtbl = {
        .write = [](void* x, Slice<const u8> buf) { return ((X*)(x))->write(buf); },
        .flush = [](void* x) { return ((X*)(x))->flush(); },
    };
    return vtbl;
  }

 public:
  struct Dyn;
  auto write_all(this Dyn self, Slice<const u8> buf) -> Result<>;
  auto write_str(this Dyn self, Str buf) -> Result<>;
};

struct Read::Dyn : public Read {
  const VTbl* _vtbl;
  void* _impl;

 public:
  template <trait::not_<Dyn> X>
  Dyn(X& x) : _vtbl{&Read::vtbl<X>()}, _impl{&x} {}

  auto read(Slice<u8> buf) -> Result<usize> {
    return (_vtbl->read)(_impl, buf);
  }
};

struct Write::Dyn : public Write {
  const VTbl* _vtbl;
  void* _impl;

 public:
  template <trait::not_<Dyn> X>
  Dyn(X& x) : _vtbl{&Write::vtbl<X>()}, _impl{&x} {}

  auto write(Slice<const u8> buf) -> Result<usize> {
    return (_vtbl->write)(_impl, buf);
  }

  auto flush() -> Result<> {
    return (_vtbl->flush)(_impl);
  }
};

}  // namespace sfc::io
