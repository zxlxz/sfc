#pragma once

#include "sfc/core.h"

namespace sfc::io {

enum class ErrorKind {
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
  ErrorKind kind = ErrorKind::Other;
  int code = 0;

 public:
  static auto last_os_error() -> Error;

  static auto from_os_error(int code) -> Error;

  void fmt(auto& f) const {
    const auto s = to_str(kind);
    f.write_str(s);
  }
};

template <class T = Tuple<>>
using Result = result::Result<T, Error>;

}  // namespace sfc::io
