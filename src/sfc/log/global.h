#pragma once

#include "sfc/log/logger.h"
#include "sfc/log/backend.h"

namespace sfc::log {

auto global() -> Logger<GlobalBackend>&;

template <class... T>
void trace(fmt::fmts_t<T...> fmts, const T&... args) {
  log::global().write_fmt(Level::Trace, fmts, args...);
}

template <class... T>
void debug(fmt::fmts_t<T...> fmts, const T&... args) {
  log::global().write_fmt(Level::Debug, fmts, args...);
}

template <class... T>
void info(fmt::fmts_t<T...> fmts, const T&... args) {
  log::global().write_fmt(Level::Info, fmts, args...);
}

template <class... T>
void warn(fmt::fmts_t<T...> fmts, const T&... args) {
  log::global().write_fmt(Level::Warn, fmts, args...);
}

template <class... T>
void error(fmt::fmts_t<T...> fmts, const T&... args) {
  log::global().write_fmt(Level::Error, fmts, args...);
}

template <class... T>
void fatal(fmt::fmts_t<T...> fmts, const T&... args) {
  log::global().write_fmt(Level::Fatal, fmts, args...);
}

}  // namespace sfc::log
