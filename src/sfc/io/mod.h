#pragma once

#include "sfc/core/result.h"

namespace sfc::str {
struct Str;
}

namespace sfc::io {

enum class Error : i8 {
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

auto to_str(Error val) noexcept -> str::Str;

auto last_os_error() noexcept -> Error;

template <class T = void>
using Result = result::Result<T, Error>;

}  // namespace sfc::io
