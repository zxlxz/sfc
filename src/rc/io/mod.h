#pragma once

#include "rc/alloc.h"

namespace rc::io {

using vec::Vec;

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
  Interrupted,
  UnexpectedEof,

  Other,
};

struct Error {
  i32 _code;

  pub static auto last_os_error() -> Error;
  pub static auto from_raw_os_error(i32 code) -> Error;
  pub auto kind() const noexcept -> ErrorKind;
  pub auto name() const noexcept -> Str;

  template <class Write>
  void fmt(fmt::Formatter<Write>& formatter) const {
    const auto s = this->name();
    formatter.pad(s);
  }
};

}  // namespace rc::io
