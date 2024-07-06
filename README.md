# SFC - C++ Common Library

  `sfc` is an open-source collection of C++ code (compliant to C++20),　suitable for use in wide range of deployment environments.`sfc` offers efficient static linking support, light weight code and low runtime overhead, strong fail-safe guarantees under correct usage.`sfc` is built on the principle that thes goals are best achieved through simple code that is easy to understand and maintain.

## 1. usage

```bash
cmake -B build
cmake --build build
```

## 2. modules

- [x] core
  - [x] chr
  - [x] cmp
  - [x] fmt
  - [x] iter
  - [x] mem
  - [x] num
  - [x] ops
  - [x] option
  - [x] panicking
  - [x] ptr
  - [x] relfect
  - [x] slice
  - [x] str
  - [x] trait
  - [x] tuple
  - [x] variant
- [x] alloc
  - [x] alloc
  - [x] box
  - [x] string
  - [x] vec
- [x] backtrace
- [x] collections
  - [x] circbuf
  - [x] queue
  - [x] vecmap
  - [x] vecset
- [x] env
- [x] ffi
- [x] fs
  - [x] file
  - [x] meta
  - [x] path
- [x] geo
  - [x] vector
  - [x] angle
  - [x] line
  - [x] point
  - [x] polygon
  - [x] trans
  - [x] projection
- [x] io
  - [x] file
  - [x] stdio
- [x] log
- [x] math
  - [x] ndview
  - [x] ndarray
- [x] serde
  - [x] ser
  - [x] des
  - [x] json
- [x] sync
  - [x] atomic
  - [x] mutex
  - [x] condvar
- [x] test
  - [x] unittest
- [x] thread
- [x] time
  - [x] datetime
  - [x] duration
  - [x] instant
