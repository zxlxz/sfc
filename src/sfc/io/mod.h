#pragma once

#include "sfc/alloc.h"

namespace sfc::io {

struct DynRead {
  class Self;
  Self& _self;
  Result<usize> (*_read)(Self&, Slice<u8> buf);

 public:
  template <class X>
  static auto of(X& x) -> DynRead {
    return DynRead{dyn::Impl{x}, dyn::Fn<&X::read>{}};
  }

 public:
  auto read(Slice<u8> buf) -> Result<usize>;
  auto read_exact(Slice<u8> buf) -> Result<>;
  auto read_to_end(List<u8>& buf) -> Result<usize>;
  auto read_to_string(String& buf) -> Result<usize>;
};

struct DynWrite {
  class Self;
  Self& _self;
  Result<usize> (*_write)(Self&, Slice<const u8> buf);
  Result<> (*_flush)(Self&){nullptr};

 public:
  template <class X>
  static auto of(X& x) -> DynWrite {
    if constexpr (requires { &X::flush; }) {
      return DynWrite{dyn::Impl{x}, dyn::Fn<&X::write>{}, dyn::Fn<&X::flush>{}};
    } else {
      return DynWrite{dyn::Impl{x}, dyn::Fn<&X::write>{}, nullptr};
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
    return DynRead::of(self).read_exact(buf);
  }

  auto read_to_end(this auto& self, List<u8>& buf) -> Result<usize> {
    return DynRead::of(self).read_to_end(buf);
  }

  auto read_to_string(this auto& self, String& buf) -> Result<usize> {
    return DynRead::of(self).read_to_string(buf);
  }
};

struct Write {
  auto write_all(this auto& self, Slice<const u8> buf) -> Result<> {
    return DynWrite::of(self).write_all(buf);
  }

  auto write_str(this auto& self, Str buf) -> Result<> {
    return DynWrite::of(self).write_str(buf);
  }
};

struct SeekFrom {
  enum Kind { Start = 0, Current = 1, End = 2 };
  Kind _tag{};
  i64 offset;
};

}  // namespace sfc::io
