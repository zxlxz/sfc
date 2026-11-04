# AGENTS.md

SFC is a zero-STL, Rust-inspired foundational library for modern C++23 applications focused on safety, determinism, and performance – built from scratch so you control every byte.

## Layout

- `src/sfc/<mod>/`: one directory per module
- `src/sfc/sys/{posix,windows}/`: platform implementations
- `cmake/`: shared CMake configs
- `build/`: CMake output, don't edit
- Each module has `<mod>.h` (header), `<mod>.cc` (source), `<mod>.cxx` (unit tests)

Module directories:

```
cmake/
src/
  sfc/
    alloc/
    app/
    collections/
    core/
    env/
    ffi/
    fs/
    io/
    log/
    serde/
    sync/
    test/
    thread/
    time/
    sys/
      posix/
      windows/
build/
```

## Build and test

```shell
cmake -B build                # configure
cmake --build build           # build
```

```shell
# run all tests
./build/bin/sfc-test

# list tests (gtest format)
./build/bin/sfc-test --gtest_list_tests

# run one test: sfc::<mod>::test.<name>
./build/bin/sfc-test --gtest_filter sfc::alloc::test.global_grow
```

## Conventions

- Don't include C++ standard headers or use `std::` types and functions.
- Unit tests go in `<mod>.cxx`, in namespace `sfc::<mod>::test`.

```cpp
#include "sfc/test/test.h"

namespace sfc::io::test {   // replace `io` with your module
  SFC_TEST(buf_read) {      // defines a test
    sfc::assert_eq(a, b);   // a == b
    sfc::assert_ne(a, b);   // a != b
  }
}
```
