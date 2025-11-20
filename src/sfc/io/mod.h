#pragma once

#include "sfc/core/result.h"

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

struct Error {
  ErrorKind kind = ErrorKind::Other;
  int code = 0;

 public:
  static auto last_os_error() -> Error;

  static auto from_os_error(int code) -> Error;

  void fmt(auto& f) const {
    f.write_val(kind);
  }
};

template <class T = void>
using Result = result::Result<T, Error>;

}  // namespace sfc::io
