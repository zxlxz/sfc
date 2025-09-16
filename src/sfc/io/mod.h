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

struct Read {
  auto read_exact(this auto& self, Slice<u8> buf) -> Result<> {
    while (!buf.is_empty()) {
      const auto read_res = Result{self.read(buf)};
      if (read_res.is_err()) {
        return read_res.unwrap_err();
      }

      if (const auto cnt = read_res.unwrap()) {
        buf = buf.slice(cnt);
      } else {
        return Error{ErrorKind::UnexpectedEof, 0};
      }
    }

    return Tuple{};
  }

  auto read_to_end(this auto& self, Vec<u8>& buf, usize buf_len = 256) -> Result<usize> {
    const auto old_len = buf.len();
    while (true) {
      buf.reserve(buf_len);

      const auto read_res = Result{self.read({buf.as_mut_ptr() + buf.len(), buf_len})};
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

  auto read_to_string(this auto& self, String& buf) -> Result<usize> {
    return self.read_to_end(buf.as_mut_vec());
  }

  auto read_bytes(this auto& self, auto& dst) -> Result<> {
    auto res = self.read(mem::as_bytes_mut(dst));
    if (res.is_err()) {
      return res.unwrap_err();
    }
    return Tuple{};
  }
};

struct Write {
  auto write_all(this auto& self, Slice<const u8> buf) -> Result<usize> {
    const auto old_len = buf.len();

    while (!buf.is_empty()) {
      const auto write_res = Result{self.write(buf)};
      if (write_res.is_err()) {
        return write_res.unwrap_err();
      }

      if (const auto cnt = write_res.unwrap()) {
        buf = buf.slice(cnt);
      } else {
        break;
      }
    }
    return old_len - buf.len();
  }

  auto write_str(this auto& self, Str buf) -> Result<usize> {
    return self.write_all(buf.as_bytes());
  }
};

}  // namespace sfc::io
