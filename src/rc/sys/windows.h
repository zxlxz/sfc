#pragma once

#include "rc/alloc.h"
#include "rc/time.h"

#include "rc/fs/mod.h"
#include "rc/io/mod.h"

namespace rc::sys::windows {

#ifndef _rc_SYS_WINDOWS_IMPL_
struct SRWLOCK;
struct CONDITION_VARIABLE;
#endif

struct Handle {
  void* _raw;

  explicit Handle(void* raw) noexcept;
  Handle(Handle&& other) noexcept;
  ~Handle();

  operator void*() const noexcept { return _raw; }
};

namespace alloc {

using namespace rc::alloc;

struct System {
  static auto alloc(Layout layout) -> void*;
  static auto alloc_zeroed(Layout layout) -> void*;
  static auto realloc(void* p, Layout layout, usize new_size) -> void*;
  static auto dealloc(void* p, Layout layout) -> void;
};
}  // namespace alloc

namespace panicking {
auto panic(Str s) -> void;
}

namespace time {
using namespace rc::time;
auto get_instant_now() -> time::Instant;
auto get_system_now() -> time::SystemTime;
}  // namespace time

namespace os {
auto last_error() -> i32;
}

namespace env {
auto var(Str key) -> String;
auto home_dir() -> String;
auto current_dir() -> String;
auto current_exe() -> String;
auto set_current_dir(Str path) -> void;
}  // namespace env

namespace io {

using rc::io::Error;
using rc::io::ErrorKind;

struct Stdin {
  static auto read(Slice<u8> buf) -> usize;
};

struct Stdout {
  static auto write(Slice<const u8> buf) -> usize;
};

struct Stderr {
  static auto write(Slice<const u8> buf) -> usize;
};

auto get_err_kind(i32 code) -> ErrorKind;

}  // namespace io

namespace fs {

using rc::fs::SeekFrom;

struct File {
  Handle _handle;
  auto read(Slice<u8> buf) -> usize;
  auto write(Slice<const u8> buf) -> usize;
  auto seek(SeekFrom from) -> usize;
  auto size() const -> usize;
};

struct OpenOptions {
  bool _read = false;
  bool _write = false;
  bool _append = false;
  bool _truncate = false;
  bool _create = false;
  bool _create_new = false;

  auto get_access_mode() const -> u32;
  auto get_create_mode() const -> u32;
  auto open(Str path) const -> File;
};

auto exists(Str p) -> bool;
auto is_dir(Str p) -> bool;
auto is_file(Str p) -> bool;

auto copy(Str from, Str to) -> void;
auto rename(Str from, Str to) -> void;

auto create_dir(Str p) -> void;
auto remove_file(Str p) -> void;
auto remove_dir(Str p) -> void;

}  // namespace fs

namespace thread {

struct Thread {
  void* _handle;

  static auto spawn(boxed::FnBox<void()> f) -> Thread;

  static auto sleep(time::Duration ms) -> void;
  static auto yield_now() -> void;
  auto join() -> void;
};

}  // namespace thread

namespace sync {

struct Mutex {
  SRWLOCK* _raw = nullptr;

  Mutex();
  ~Mutex();
  Mutex(Mutex&& other) noexcept;

  auto lock() -> void;
  auto unlock() -> void;
  auto trylock() -> bool;
};

struct CondVar {
  CONDITION_VARIABLE* _raw;

  CondVar();
  ~CondVar();
  CondVar(CondVar&& other) noexcept;

  auto wait(Mutex& mtx) -> void;
  auto wait_timeout(Mutex& mtx, time::Duration dur) -> bool;

  auto notify_one() -> void;
  auto notify_all() -> void;
};

}  // namespace sync

namespace term {
auto columns() -> usize;
}

}  // namespace rc::sys::windows

namespace rc::sys {
#ifdef _WIN32
using namespace ::rc::sys::windows;
#endif
}  // namespace rc::sys
