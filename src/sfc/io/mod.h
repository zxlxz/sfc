#pragma once

#include "sfc/alloc.h"

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

struct DynRead {
  class Self;
  Self& _self;
  auto (*_read)(Self&, Slice<u8> buf) -> Result<usize>;

 public:
  template <class X>
  explicit DynRead(X& x) : _self{dyn::cast<Self>(x)}, _read{dyn::Fn<&X::read>{}} {}

 public:
  auto read(Slice<u8> buf) -> Result<usize>;
  auto read_exact(Slice<u8> buf) -> Result<>;
  auto read_to_end(List<u8>& buf) -> Result<usize>;
  auto read_to_string(String& buf) -> Result<usize>;
};

struct DynWrite {
  class Self;
  Self& _self;
  auto (*_write)(Self&, Slice<const u8> buf) -> Result<usize>;
  auto (*_flush)(Self&) -> Result<>{nullptr};

 public:
  template <class X>
  explicit DynWrite(X& x) : _self{dyn::cast<Self>(x)}, _write{dyn::Fn<&X::write>{}} {
    if constexpr (requires { &X::flush; }) {
      this->_flush = dyn::Fn<&X::flush>{};
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

}  // namespace sfc::io
