#pragma once

#include "sfc/alloc.h"

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

template <class T>
struct Read {
  T _inn;

 public:
  auto read(Slice<u8> buf) -> Result<usize> {
    return _inn.read(buf);
  }

  auto read_to_end(Vec<u8>& buf, usize buf_len = 256) -> Result<usize> {
    const auto old_len = buf.len();
    while (true) {
      buf.reserve(buf_len);

      const auto read_res = this->read({buf.as_mut_ptr() + buf.len(), buf_len});
      if (read_res.is_err()) {
        return read_res.unwrap_err();
      }

      if (const auto cnt = read_res.unwrap()) {
        buf.set_len(buf.len() + cnt);
      } else {
        break;
      }
    }
    return buf.len() - old_len;
  }

  auto read_to_string(String& buf) -> Result<usize> {
    return this->read_to_end(buf.as_mut_vec());
  }

  auto read_bytes(auto& dst) -> Result<> {
    auto res = this->read(mem::as_bytes_mut(dst));
    if (res.is_err()) {
      return res.unwrap_err();
    }
    return Tuple{};
  }
};

template <class T>
struct Write {
  T _inn;

 public:
  auto write(Slice<const u8> buf) -> Result<usize> {
    return _inn.write(buf);
  }

  auto write_all(Slice<const u8> buf) -> Result<usize> {
    const auto old_len = buf.len();

    while (!buf.is_empty()) {
      const auto write_res = this->write(buf);
      if (write_res.is_err()) {
        return write_res.unwrap_err();
      }

      const auto cnt = write_res.unwrap();
      if (cnt == 0) {
        break;
      }
      buf = buf[{cnt, buf.len()}];
    }
    return old_len - buf.len();
  }

  auto write_str(Str buf) -> Result<usize> {
    return this->write_all(buf.as_bytes());
  }
};

template <class T>
Read(T) -> Read<T>;

template <usize N>
Read(const u8 (&)[N]) -> Read<Slice<const u8>>;

}  // namespace sfc::io
