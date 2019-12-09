#pragma once

#include "rc/alloc.h"
#include "rc/time.h"

namespace rc::log {

using boxed::FnBox;
using time::Duration;

enum class Level {
  Trace,
  Debug,
  Info,
  Warn,
  Error,
  Fatal,
  User,
};

struct Entry {
  Level _level;
  Duration _time;
  Slice<const u8> _msg;
};

struct Logger;

template <class... Args>
auto write(Level level, Str sfmt, const Args&... args) -> void;

template <class... T>
inline auto trace(const Str& sfmt, const T&... args) -> void {
  log::write(Level::Trace, sfmt, args...);
}

template <class... T>
inline auto debug(const Str& sfmt, const T&... args) -> void {
  log::write(Level::Debug, sfmt, args...);
}

template <class... T>
inline auto info(const Str& sfmt, const T&... args) -> void {
  log::write(Level::Info, sfmt, args...);
}

template <class... T>
inline auto warn(const Str& sfmt, const T&... args) -> void {
  log::write(Level::Warn, sfmt, args...);
}

template <class... T>
inline auto error(const Str& sfmt, const T&... args) -> void {
  log::write(Level::Error, sfmt, args...);
}

template <class... T>
inline auto fatal(const Str& sfmt, const T&... args) -> void {
  log::write(Level::Fatal, sfmt, args...);
}

template <class... T>
inline auto user(const Str& sfmt, const T&... args) -> void {
  log::write(Level::User, sfmt, args...);
}

}  // namespace rc::log
