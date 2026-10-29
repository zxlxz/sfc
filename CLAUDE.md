# CLAUDE.md

Guidance for AI coding agents working in this repository.

## Project

SFC (Standard Foundation for C++): a zero-STL, Rust-inspired C++23 foundational library — a curated port of Rust's std. One static library target `sfc` plus the `sfc-test` runner. GPLv3.

Not all of Rust std is ported:
- `borrow`, `pin`, `cell`: don't fit C++.
- `future`: too complex, not planned.
- `net`: marked `NOT_IMPL_YET` in the module map.

## Commands

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug   # configure
cmake --build build                        # build libsfc.a and sfc-test
ctest --test-dir build                     # run all tests
```

Test runner (Windows: `sfc-test.exe`):

```bash
build/src/sfc/sfc-test --gtest_filter='sfc::alloc::test.global_shrink'  # one test
build/src/sfc/sfc-test --gtest_filter='sfc::alloc::test.*'              # one suite
build/src/sfc/sfc-test --gtest_list_tests --gtest_output=xml:tests.xml  # list tests to a file
```

- Filter pattern is `<suite>.<test>` with the full suite name (`sfc::alloc::test`), or `<suite>.*`. A bare test name (`global_shrink`) and `<suite>::*` both match 0 tests.
- `--gtest_output` only applies to `--gtest_list_tests`. Use the `xml:<file>` form; the bare form printed nothing when piped.

## Hard constraints

- **No C++ standard library headers.** Only C headers (`stdio.h`, `string.h`, …) and OS headers (`Windows.h`, `pthread.h`, …) may be `#include`d, and only under `sys/`. Every other facility (containers, type traits, iterators, formatting) lives in `src/sfc/`; check `core/` and `alloc/` before writing a new primitive.
- **No exceptions in library code.** Report failures with `Result<T,E>` / `Option<T>`, or `panic`. The test runner itself uses try/catch to catch panics (`src/sfc/test/test.cc`).
- **Warnings stay clean.** `-Wall -Wextra -Wconversion` is set for all targets (`src/CMakeLists.txt`); new code must not add warnings.

## Architecture

Layers, bottom to top; each layer only uses the ones below it. Paths are relative to `src/sfc/`.

1. **`sys/`** — raw OS layer. Platform-neutral contracts (`sys/io.h`, `sys/fs.h`, `sys/backtrace.h`, …) with implementations in `sys/posix/*.cc` (Linux/macOS) or `sys/windows/*.cc` (Windows). CMake compiles exactly one platform directory. Only `sys/` calls OS APIs; higher modules call `sys::*`. Keep posix and windows in API parity.
2. **`core/`** — foundation types used everywhere (`Option`, `Result`, `Tuple`, `Variant`, `Slice`, `Str`, `iter`, `fmt`, `hash`, `panic`, … see module map). Contract headers also live here: `core/io.h` (`io::Error`), `core/time.h`.
3. **`alloc/`** — allocator trait (`Allocator`, `System`, alias `Global`, `Layout`), owning types `Box`, `String`, `List<T>` (the vector replacement), and `raw_buf`.
4. **`sync/`** — `sync/atomic.h` (`Atomic<T>`, `Ordering`) and `sync/arc.h` (`Arc<T>`). Shared ownership is always `Arc<T>` (atomic refcount); there is no non-atomic `Rc`.
5. **Everything else** — `io`, `fs`, `env`, `ffi`, `time`, `thread`, `collections`, `serde`, `app`, `log` — built on the layers above.

### File layout

- Each unit has `name.h` (interface), `name.cc` (library code, compiled into `sfc`), and `name.cxx` (tests, compiled into `sfc-test`). The extension selects the target.
- `core/` and `alloc/` are flat: `core/<unit>.*`, `alloc/<unit>.*`.
- Other modules are directories `<mod>/<unit>.*` with an umbrella header `<mod>.h` (e.g. `io.h`); `core.h` aggregates `core/`.
- Namespaces mirror Rust modules: `sfc::<name>` ports Rust module `<name>`.
- Sources are globbed by `target_source_files` (`cmake/utils.cmake`, `CONFIGURE_DEPENDS`), so new files are picked up automatically; no CMakeLists edit is needed.

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
| `std::io` | `sfc::io` | `Error`, `Read`, `Write`, `SeekFrom`, `File`, `BufReader`, `BufWriter`, `Stdout`, `Stderr`, `Pod` | `stdout`, `stderr`, `print`, `println` |
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

Custom gtest-like framework in `src/sfc/test/`. Tests live in the `.cxx` next to the code:

```cpp
#include "sfc/test/test.h"

namespace sfc::alloc::test {

SFC_TEST(global_grow) {  // registered at static init
  sfc::assert_eq(a, b);  // assert_eq/ne/lt/le/gt/ge, assert_flt_eq (default epsilon 1e-12)
}

}  // namespace sfc::alloc::test
```

- Suite = enclosing namespace (`sfc::alloc::test`); test name = macro argument (`global_grow`).
- A failed assert panics with its source location; the runner catches the panic and reports the test as failed.

## Style

- Format with clang-format (`.clang-format`: Google-based, 120 columns). **`SortIncludes: false`**: include order is deliberate; never re-sort.
- Trailing return types `auto f() -> T`; lowercase module and function names; PascalCase types.
- Slice ranges are `s[{begin, end}]`; `$` means end, e.g. `s[{2, $}]`.
- Formattable types define `void fmt(auto& f) const`; format with `"{}"` via `io::println` or `f.write_fmt`. No `<format>`, no iostreams.

## Verification

Before finishing a change:
1. `cmake --build build` must add no new warnings (`-Wall -Wextra -Wconversion`).
2. `ctest --test-dir build`, or a filtered `sfc-test` run while iterating.
3. If you touched `sys/`, mirror the change in both `sys/posix` and `sys/windows`.

CI (`.github/workflows/cmake-single-platform.yml`): Ubuntu, gcc-15, Release, `ctest`. Only posix is built and tested there.
