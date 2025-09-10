#pragma once

#include "sfc/alloc.h"

namespace sfc::io {

enum class ErrorKind : u8 {
  NotFound,
  PermissionDenied,
  ConnectionRefused,
  ConnectionReset,
  ConnectionAborted,
  NotConnected,
  AddrInUse,
  AddrNotAvailable,
  BrokenPipe,
  AlreadyExists,
  WouldBlock,
  InvalidInput,
  InvalidData,
  TimedOut,
  WriteZero,
  UnexpectedEof,
  Interrupted,
  Other,
};

auto to_str(ErrorKind kind) -> Str;

struct Error {
  using Kind = ErrorKind;

  Kind _kind;
  int _code = 0;

 public:
  static auto last_os_error() -> Error;

  static auto from_os_error(int code) -> Error;

  auto kind() const -> ErrorKind {
    return _kind;
  }

  void fmt(auto& f) const {
    f.write(to_str(_kind));
  }
};

template <class T = Dummy>
using Result = result::Result<T, Error>;

}  // namespace sfc::io
