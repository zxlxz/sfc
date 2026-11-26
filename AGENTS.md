# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

SFC is a zero-STL, Rust-inspired foundational library for modern C++23, focused on safety, determinism, and performance.

## Build and Test

```shell
cmake -B build                # configure
cmake --build build           # build

./build/bin/sfc-test                                                # all tests
./build/bin/sfc-test --gtest_list_tests                             # list tests (XML)
./build/bin/sfc-test --gtest_filter 'sfc::alloc::test.*'            # one module
./build/bin/sfc-test --gtest_filter 'sfc::alloc::test.global_grow'  # one test

```

- A filter is `sfc::<mod>::test.<name>` or `sfc::<mod>::test.*`. Join several with `:` or `;`.
- A bare test name matches nothing, and the run still exits 0.
- No formatter or linter is wired into the build. `.clang-format` is the style source.

## Layout

- `src/sfc/<mod>/`: one directory per module. A unit is `<x>.h` + `<x>.cc`, with tests in `<x>.cxx`.
- `src/sfc/<mod>.h`: umbrella header. Users include `"sfc/<mod>.h"`.
- `src/sfc/sys/{posix,windows}/`: platform implementations.
- `cmake/`: shared CMake configs. `build/`: CMake output, don't edit.

Every `.cc` under `src/sfc/` is globbed into the `sfc` library and every `.cxx` into `sfc-test`, so new files and modules need no CMake edits.

## Architecture

Layers, each depending only on those above it:

1. `core`: integers (`i32`, `usize`, ...), `Str`/`Slice`, `Option`/`Result`, `Tuple`/`Variant`, iterators, `fmt`, `panic`.
2. `alloc`: allocator interface over `mem::Layout`, plus `Box`, `String`, `List`.
3. Feature modules: `collections`, `io`, `fs`, `sync`, `thread`, `time`, `env`, `ffi`, `log`, `serde`, `app`.
4. `sys`: raw OS layer, and the only place that includes C/OS headers. `sys/<area>.h` picks the `posix/` or `windows/` implementation, so callers write `sys::File` with no platform branches.

## Conventions

- No C++ standard headers, no `std::`.
- Types live in `sfc::<mod>::` and are re-exported into `sfc` with `using`.
- Rust-flavoured naming: `new_()` constructors, `as_*` views, `into_*` on `&&`, `_member` fields, concepts with a trailing underscore (`same_`, `int_`).
- Errors are `Result<T, E>` / `Option<T>`. `panic` unwinds by throwing, so exceptions must stay enabled.
- A type is printable via `void fmt(auto& f) const`; enums print through ADL `to_str`.
- Dynamic dispatch is hand-rolled: `void* _self` plus a `static constexpr sVTBL<X>` of function pointers, built from lambdas in a templated constructor (see `io::Read`). The lambda-to-function-pointer conversion is what constrains `X`, so no `requires` clause is needed.
- Serde: `serde::Serialize<T>` / `serde::Deserialize<T>` fall back to the members `T::serialize` / `T::deserialize`.

## Design

- Compare with Rust's standard library for similar patterns and idioms.
- Don't read Rust source or try to build Rust code. Focus on the design patterns and idioms.

## Unit tests


- Tests go in `<x>.cxx`, inside `namespace sfc::<mod>::test`. The namespace is the suite, so it sets the filter name.
- Assertions: `assert_eq`, `assert_ne`, `assert_lt`, `assert_le`, `assert_gt`, `assert_ge`, `assert_flt_eq`, `assert_flt_ne`.

- Test example:

```cpp
#include "sfc/test/test.h"

namespace sfc::io::test {   // replace `io` with your module
  SFC_TEST(buf_read) {
    sfc::assert_eq(a, b);
    sfc::assert_ne(a, b);
  }
}
```
