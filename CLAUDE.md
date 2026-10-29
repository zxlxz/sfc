# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

SFC (Standard Foundation for C++): a zero-STL, Rust-inspired C++23 foundational library — a curated port of Rust's std. Single static `sfc` library target plus a custom test runner. GPLv3.

Scope: not all of Rust std is ported.
  - `borrow`, `pin`, `cell`: not fit c++
  - `future`: too complex, not plan to impl.

## Commands

```bash
# build
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# run test
./build/src/sfc/sfc-test.exe          # Windows
ctest --test-dir build                # equivalent

# List tests
./build/src/sfc/sfc-test.exe --gtest_list_tests

# Run one test — filter MUST be "suite.testname"
./build/src/sfc/sfc-test.exe --gtest_filter='sfc::alloc::test.global_shrink'
```

**Filter quirk:** a bare test name (`--gtest_filter=global_shrink`) matches 0 tests — the pattern must start with the full suite name or be a wildcard. Use `sfc::<module>::test.<name>` or `sfc::<module>::test::*`.

## Hard constraints

- **No C++ standard library headers.** Only C library headers (`stdio.h`, `stdlib.h`, `string.h`, …) and OS headers (`Windows.h`, `pthread.h`, …) may be `#include`d in sys/*. Every C++ facility (containers, type traits, iterators, formatting) is defined inside `src/sfc/` — check `core/` for an existing primitive before writing one.
- **Exceptions are avoided** in library code; use `Result<T,E>` / `Option<T>` + `panic`. (The test runner itself uses try/catch to catch panics.)
- Warnings are errors-in-spirit: `-Wall -Wextra -Wconversion` is always on; keep new code clean under `-Wconversion`.

## Architecture

Layering, bottom to top; each layer only calls the ones below it:

1. **`sys/`** — raw OS layer. Platform-neutral contract headers (`sys/io.h`, `sys/fs.h`, `sys/backtrace.h`, …); implementations in `sys/posix/*.cc` or `sys/windows/*.cc`. CMake compiles exactly one platform directory (`LINUX OR APPLE` → posix, `WIN32` → windows). Higher modules never touch OS APIs directly — they call `sys::*`.
2. **`core/`** — foundation types used by everything (`Option`, `Result`, `Tuple`, `Variant`, `Slice`, `Str`, `iter`, `fmt`, `hash`, `panic`, … — full list in the module map). Contract headers also live here: `core/io.h` (`io::Error`), `core/time.h`.
3. **`alloc/`** — memory: allocator trait (`Allocator`, `System`, alias `Global`; `Layout`), owning types `Box`, `String`, `List<T>` (the vector replacement), `raw_buf`.
4. **`sync/`** — synchronization primitives, including `sync/atomic.h` (`Atomic<T>`, `Ordering`) and shared ownership `sync/arc.h` (`Arc<T>`).
5. **Everything else** — `io`, `fs`, `env`, `ffi`, `time`, `thread`, `collections`, `serde`, `app`, `log` — built on core + alloc + sys + sync where needed.

Shared ownership is always `Arc<T>` (`sync/arc.h`, atomic refcount) — there is no non-atomic `Rc`.

### Module file convention

sfc namespaces mirror Rust modules: `sfc::<name>` ports Rust module `<name>`. File layout per unit: `name.h` (header) + `name.cc` (impl) + `name.cxx` (tests), named after the unit:

- `core/` and `alloc/` units are flat: `sfc/core/<mod>.{h,cc,cxx}`, `sfc/alloc/<mod>.{h,cc,cxx}`.
- Every other module is a directory: `sfc/<mod>/<unit>.{h,cc,cxx}`, with an umbrella header `sfc/<mod>.h` (e.g. `sfc/io.h`); `sfc/core.h` aggregates core.
- Sources are glob-recursed via `target_source_files` in `cmake/utils.cmake` — new files are picked up on reconfigure, no CMakeLists edit needed.

### Rust ↔ sfc module map

**Rust core**

| Rust | sfc namespace | structs | functions |
|------|---------------|---------|-----------|
| `tuple` | `sfc::tuple` | `Tuple<T...>` | — |
| `enum` | `sfc::variant` | `Variant<T...>`, `Union<T...>` | — |
| `core::num` | `sfc::num` | `Int<T>`, `Float<T>` (aliases `i32`, `u32`, `usize`, …) | — |
| `core::char` | `sfc::chr` | `Chars`, `U16Chars` | — |
| `core::ops` | `sfc::ops` | `Fn<R(T...)>`, `Range`, `IntSeq` | `declval` |
| `core::ptr` | `sfc::ptr` | `Unique<T>` | `null`, `cast`, `read`, `write`, `drop_in_place`, `copy`, `copy_nonoverlapping` |
| `core::mem` | `sfc::mem` | `Layout` | `move`, `drop`, `swap`, `assign`, `replace`, `take` |
| `core::option` | `sfc::option` | `Option<T>`, `None` | — |
| `core::result` | `sfc::result` | `Result<T,E>`, `Ok`, `Err` | — |
| `core::slice` | `sfc::slice` | `Slice<T>`, `Windows`, `Chunks` | — |
| `core::str` | `sfc::str` | `Str`, `Pattern`, `Searcher` | — |
| `core::fmt` | `sfc::fmt` | `Formatter`, `Fmts`, `Args` | `write`, `writeln` |
| `core::future` | `NOT_IMPL_YET` | — | — |

**Rust alloc**

| Rust | sfc namespace | structs | functions |
|------|---------------|---------|-----------|
| `std::alloc::alloc` | `sfc::alloc` | `Allocator`, `System` (alias `Global`), `Layout` | — |
| `std::alloc::boxed` | `sfc::boxed` | `Box<T>`, `Box<R(T...)>` | — |
| `std::alloc::vec` | `sfc::list` | `List<T, A>`, `List::Drain` | — |
| `std::alloc::string` | `sfc::string` | `String` | — |

**Rust std**

| Rust | sfc namespace | structs | functions |
|------|---------------|---------|-----------|
| `std::collections` | `sfc::collections` | `HashMap`, `HashSet`, `Queue` | — |
| `std::sync` | `sfc::sync` | `Arc`, `Mutex`, `ReentrantLock`, `Condvar`, `mpmc::Channel`, `RingBuf` | — |
| `std::thread` | `sfc::thread` | `Thread`, `JoinHandle`, `Builder`, `Scope` | `spawn`, `current`, `current_id`, `yield_now`, `sleep`, `sleep_ms` |
| `std::time` | `sfc::time` | `Instant`, `SystemTime`, `DateTime` | — |
| `std::io` | `sfc::io` | `Error`, `Read`, `Write`, `SeekFrom`, `File`, `BufReader`, `BufWriter`, `Stdout`, `Stderr` | `stdout`, `stderr`, `print`, `println` |
| `std::path` | `sfc::fs` | `Path`, `PathBuf` | — |
| `std::fs` | `sfc::fs` | `File`, `Metadata`, `OpenOptions`, `DirEntry` | `read`, `write`, `metadata`, `create_dir` |
| `std::env` | `sfc::env` | — | `var`, `set_var`, `remove_var`, `current_dir`, `home_dir`, `temp_dir` |
| `std::ffi` | `sfc::ffi` | `CStr`, `CString`, `WStr`, `WCString`, `Library` | — |
| `std::net` | `NOT_IMPL_YET` | — | — |

**Rust crates**

| Rust | sfc port |
|------|----------|
| `serde_json` | `sfc::serde::json` |
| `clap` | `sfc::app::clap` |
| `log` | `sfc::log` (`logger`, `backend`) |

### Tests

Custom gtest-like framework in `src/sfc/test/`. Tests live next to the code in `.cxx` files:

```cpp
#include "sfc/test/test.h"

namespace sfc::alloc::test {

SFC_TEST(global_grow) {   // auto-registers via static init; suite = enclosing namespace
  sfc::assert_eq(a, b);   // assert_eq/ne/lt/le/gt/ge, assert_flt_eq; aborts with context on failure
}

}  // namespace sfc::alloc::test
```

The runner (`sfc-test`) derives suite/test names from the registration type name (`sfc::alloc::test::_global_grow` → suite `sfc::alloc::test`, test `global_grow`) and supports gtest-style flags (`--gtest_filter`, `--gtest_list_tests`, `--gtest_output=<file>` for XML).

### Formatting & style

- clang-format: Google-based, 120 columns, **`SortIncludes: false`** — include order is deliberate, never re-sort.
- Rust-flavored C++ style: `auto f() -> T` trailing returns, lowercase module/function names, PascalCase types, slice ranges written `s[{begin, end}]` with `$` meaning "to end" (e.g. `s[{2, $}]`).
- Formattable types opt in via a `void fmt(auto& f) const` member; formatting uses `"{}"` placeholders (`io::println`, `f.write_fmt`) — no `<format>`, no iostreams.

## CI

GitHub Actions (`.github/workflows/cmake-single-platform.yml`): ubuntu, gcc-15, Release, `ctest`. Keep `sys/posix` and `sys/windows` implementations in API parity — CI only exercises posix.
