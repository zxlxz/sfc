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

  auto kind() const noexcept -> ErrorKind;
  auto raw_os_err() const noexcept -> ErrCode;
  auto to_str() const noexcept -> Str;

  void fmt(auto& f) const {
    const auto s = this->to_str();
    f.write_str(s);
  }
};

template <class T = void>
using Result = result::Result<T, Error>;

}  // namespace sfc::io
