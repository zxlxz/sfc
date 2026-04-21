#pragma once

#include "sfc/core/result.h"

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

auto from_raw_os_error(i32 err) noexcept -> Error;

auto last_os_error() noexcept -> Error;

template <class T = void>
using Result = result::Result<T, Error>;

}  // namespace sfc::io
