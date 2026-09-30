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
  class Dyn;
  auto read_exact(this Dyn self, Slice<u8> buf) -> Result<>;
  auto read_to_end(this Dyn self, List<u8>& buf) -> Result<usize>;
  auto read_to_string(this Dyn self, String& buf) -> Result<usize>;
};

struct Write {
  class Dyn;
  auto write_all(this Dyn self, Slice<const u8> buf) -> Result<>;
  auto write_str(this Dyn self, Str buf) -> Result<>;
};

class Read::Dyn : public Read {
  struct VTbl {
    Result<usize> (*read)(void*, Slice<u8>);

    template <class X>
    static auto of() -> const VTbl& {
      static const auto vtbl = VTbl{
          .read = [](void* x, Slice<u8> buf) { return ((X*)(x))->read(buf); },
      };
      return vtbl;
    }
  };
  const VTbl& _vtbl;
  void* _impl;

 public:
  template <trait::not_<Dyn> X>
  Dyn(X& x) : _vtbl{VTbl::of<X>()}, _impl{&x} {}

  auto read(Slice<u8> buf) -> Result<usize> {
    return (_vtbl.read)(_impl, buf);
  }
};

class Write::Dyn : public Write {
  struct VTbl {
    Result<usize> (*write)(void*, Slice<const u8>);
    Result<> (*flush)(void*);

    template <class X>
    static auto of() -> const VTbl& {
      static const auto vtbl = VTbl{
          .write = [](void* x, Slice<const u8> buf) { return ((X*)(x))->write(buf); },
          .flush = [](void* x) { return ((X*)(x))->flush(); },
      };
      return vtbl;
    }
  };
  const VTbl& _vtbl;
  void* _impl;

 public:
  template <trait::not_<Dyn> X>
  Dyn(X& x) : _vtbl{VTbl::of<X>()}, _impl{&x} {}

  auto write(Slice<const u8> buf) -> Result<usize> {
    return (_vtbl.write)(_impl, buf);
  }

  auto flush() -> Result<> {
    return (_vtbl.flush)(_impl);
  }
};

}  // namespace sfc::io
