
#pragma once

#include "sfc/core.h"

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
  HostUnreachable,
  NetworkUnreachable,
  ConnectionAborted,
  NotConnected,
  AddrInUse,
  AddrNotAvailable,
  NetworkDown,
  BrokenPipe,
  AlreadyExists,
  WouldBlock,
  NotADirectory,
  IsADirectory,
  DirectoryNotEmpty,
  ReadOnlyFilesystem,
  FilesystemLoop,
  StaleNetworkFileHandle,
  InvalidInput,
  InvalidData,
  TimedOut,
  WriteZero,
  StorageFull,
  NotSeekable,
  QuotaExceeded,
  FileTooLarge,
  ResourceBusy,
  ExecutableFileBusy,
  Deadlock,
  CrossesDevices,
  TooManyLinks,
  InvalidFilename,
  ArgumentListTooLong,
  Interrupted,
  Unsupported,
  UnexpectedEof,
  OutOfMemory,
  InProgress,
  Other,
};

auto to_str(Error val) noexcept -> str::Str;

auto last_os_error() noexcept -> Error;

template <class T = void>
using Result = result::Result<T, Error>;

}  // namespace sfc::io
