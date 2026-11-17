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

class Read final {
  struct VTbl {
    Result<usize> (*read)(void*, Slice<u8>);
  };

  template <class X>
  static constexpr auto vtbl = VTbl{
      .read = [](void* x, Slice<u8> buf) { return ((X*)x)->read(buf); },
  };

  void* _self;
  const VTbl* _vtbl;

 public:
  template <trait::not_<Read> X>
  Read(X& x) : _self{&x}, _vtbl{&vtbl<X>} {}

 public:
  auto read(Slice<u8> buf) -> Result<usize>;
  auto read_exact(Slice<u8> buf) -> Result<>;
  auto read_to_end(List<u8>& buf) -> Result<usize>;
  auto read_to_string(String& buf) -> Result<usize>;
};

class Write final {
  struct VTbl {
    Result<usize> (*write)(void*, Slice<const u8>);
    Result<> (*flush)(void*);
  };

  template <class X>
  static constexpr auto vtbl = VTbl{
      .write = [](void* x, Slice<const u8> buf) { return ((X*)x)->write(buf); },
      .flush = [](void* x) { return ((X*)x)->flush(); },
  };

  void* _self;
  const VTbl* _vtbl;

 public:
  template <trait::not_<Write> X>
  Write(X& x) : _self{&x}, _vtbl{&vtbl<X>} {}

 public:
  auto write(Slice<const u8> buf) -> Result<usize>;
  auto flush() -> Result<>;

  auto write_all(Slice<const u8> buf) -> Result<>;
  auto write_str(Str buf) -> Result<>;
};

class Seek final {
  struct VTbl {
    Result<u64> (*seek)(void*, SeekFrom);
  };

  template <class X>
  static constexpr auto vtbl = VTbl{
      .seek = [](void* x, SeekFrom pos) { return ((X*)x)->seek(pos); },
  };

  void* _self;
  const VTbl* _vtbl;

 public:
  template <trait::not_<Seek> X>
  Seek(X& x) : _self{&x}, _vtbl{&vtbl<X>} {}

 public:
  auto seek(SeekFrom pos) -> Result<u64>;
  auto seek_relative(i64 offset) -> Result<u64>;
  auto stream_position() -> Result<u64>;
};

struct IRead {
  auto read_exact(this auto& self, Slice<u8> buf) -> Result<> {
    return Read{self}.read_exact(buf);
  }

  auto read_to_end(this auto& self, List<u8>& buf) -> Result<usize> {
    return Read{self}.read_to_end(buf);
  }

  auto read_to_string(this auto& self, String& buf) -> Result<usize> {
    return Read{self}.read_to_string(buf);
  }
};

struct IWrite {
  auto write_all(this auto& self, Slice<const u8> buf) -> Result<> {
    return Write{self}.write_all(buf);
  }

  auto write_str(this auto& self, Str buf) -> Result<> {
    return Write{self}.write_all(buf.as_bytes());
  }
};

struct ISeek {
  auto seek_relative(this auto& self, i64 offset) -> Result<u64> {
    return Seek{self}.seek_relative(offset);
  }

  auto stream_position(this auto& self) -> Result<u64> {
    return Seek{self}.stream_position();
  }
};

}  // namespace sfc::io
