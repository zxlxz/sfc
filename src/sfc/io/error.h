#pragma once

#include "sfc/sys.h"

namespace sfc::io {

using sys::ErrCode;

enum class ErrorKind {
  Success,
  NotFound,
  PermissionDenied,
  ConnectionRefused,
  ConnectionReset,
  ConnectionAborted,
  NotConnected,
  AddrInUse,
  AddrNotAvailable,
  NetworkUnreachable,
  HostUnreachable,
  NetworkDown,
  BrokenPipe,
  AlreadyExists,
  WouldBlock,
  InvalidInput,
  InvalidData,
  InvalidOperation,
  Interrupted,
  Unsupported,
  UnexpectedEof,
  WriteZero,
  TimedOut,
  IsADirectory,
  NotADirectory,
  DirectoryNotEmpty,
  NotSeekable,
  FileTooLarge,
  ResourceBusy,
  Deadlock,
  StorageFull,
  OutOfMemory,
  InProgress,
  Other,
};

struct Error {
  ErrorKind _kind;
  ErrCode _code = 0;

 public:
  static auto from_raw_os_error(ErrCode err) noexcept -> Error;
  static auto last_os_error() noexcept -> Error;

  auto kind() const noexcept -> ErrorKind {
    return _kind;
  }

  auto raw_os_err() const noexcept -> ErrCode {
    return _code;
  }

  void fmt(auto& f) const {
    f.write_val(_kind);
  }
};

template <class T = void>
using Result = result::Result<T, Error>;

}  // namespace sfc::io
