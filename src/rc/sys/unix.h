#pragma once

#include "rc/alloc.h"
#include "rc/time.h"

#include "rc/fs/mod.h"
#include "rc/io/mod.h"

namespace rc::sys::unix {

#ifndef _rc_SYS_UNIX_IMPL_
struct pthread_mutex_t;
struct pthread_cond_t;
using pthread_t = struct pthread_st*;
#endif

struct FileDesc {
  int _raw;

  explicit FileDesc(int raw) noexcept : _raw{raw} {}

  ~FileDesc() {
    if (_raw == -1) return;
    this->drop();
  }

  operator int() const noexcept { return _raw; }

  auto drop() -> void;
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
  FileDesc _filedesc;

  auto read(Slice<u8> buf) -> usize;
  auto write(Slice<const u8> buf) -> usize;
  auto seek(SeekFrom pos) -> usize;
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
  pthread_t _raw;

  static auto spawn(boxed::FnBox<void()> f) -> Thread;
  static auto sleep(time::Duration ms) -> void;
  static auto yield_now() -> void;

  auto join() -> void;
};
}  // namespace thread

namespace sync {

struct Mutex {
  pthread_mutex_t* _raw = nullptr;

  Mutex();
  ~Mutex();
  Mutex(Mutex&& other) noexcept;

  auto lock() -> void;
  auto unlock() -> void;
  auto trylock() -> bool;
};

struct CondVar {
  pthread_cond_t* _raw;

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

}  // namespace rc::sys::unix

namespace rc::sys {
#if defined(__unix__) || defined(__APPLE__)
using namespace ::rc::sys::unix;
#endif
}  // namespace rc::sys
