#pragma once

#include "sfc/sync.h"

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

struct Error {
  ErrorKind _kind;
  int _code = 0;

 public:
  static auto last_os_error() -> Error;

  static auto from_os_error(int code) -> Error;

  auto kind() const -> ErrorKind {
    return _kind;
  }

  auto as_str() const -> Str;

  void fmt(auto& f) const {
    const auto s = this->as_str();
    f.write(s);
  }
};

template <class T = Dummy>
using Result = result::Result<T, Error>;

}  // namespace sfc::io
