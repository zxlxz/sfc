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

template <class T = void>
using Result = result::Result<T, Error>;

}  // namespace sfc::io
