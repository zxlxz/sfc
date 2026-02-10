# SFC (Standard Foundation for C++)

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Language: C++23](https://img.shields.io/badge/Language-C%2B%2B23-purple.svg)](https://isocpp.org/)
[![Platforms](https://img.shields.io/badge/Platforms-Windows%20%7C%20macOS%20%7C%20Linux-orange.svg)](https://isocpp.org/)
[![No STL](https://img.shields.io/badge/Dependency-0%20%F0%9F%9A%AB%20STL-brightgreen.svg)](#)
[![PRs Welcome](https://img.shields.io/badge/PRs-Welcome-blue.svg)](#contributing)

> A zero-STL, Rust-inspired foundational library for modern C++23 applications focused on safety, determinism, and performance – built from scratch so you control every byte.

---

## Why SFC?

The C++ standard library is large, legacy-constrained, ABI-fragile, and sometimes unpredictable across platforms. Embedded, systems, tooling, game, and low-latency domains often need:

* Deterministic behavior (no surprise allocations / hidden slow paths)
* Tight binary size and no dependency on libstdc++ / MSVCPRT
* Unified, expressive, ergonomic APIs (Rust-like `Result`, `Option`, ownership types)
* Cross-platform primitives without #ifdef sprawl

SFC re-imagines the “stdlib” surface for C++23 with explicit design: minimal unsafe surface, predictable memory, and composable primitives.

---

## ✨ Feature Highlights

| Domain | What You Get | Notes |
|--------|--------------|-------|
| Core Types | `Option<T>`, `Result<T,E>`, `Tuple`, `Variant`, iter utilities | Error handling & composition first |
| Ownership | `Box<T>`, `Rc<T>` | Deterministic lifetimes, no STL smart ptrs |
| Memory | Custom allocator hooks, slices, raw pointer helpers | Fine-grained control |
| Collections | `Vec<T>`, `VecMap<K,V>`, `BTree*`, `HashMap` | Purpose-built, STL-free |
| Concurrency | `Mutex`, `Condvar`, atomics, MPMC queue, threads | Portable & lean |
| Tasking | Lightweight async/task primitives (WIP) | Foundation for schedulers |
| I/O & FS | Paths, files, buffered stdio, platform bridges | Unified Windows / POSIX |
| Time | Steady + system clocks, duration types | Clear conversions |
| Logging | Pluggable backend-friendly logging core | Minimal formatting now, richer later |
| Ser/De | JSON, XML (early stage) | Extensible traits design |

---

## 🔍 Design Principles

1. Zero STL dependency – everything is internally defined.
2. Fail fast: panic expections and explicit `Result`/`Option` return types.
3. No hidden allocations: APIs surface ownership & lifetime.
4. Cross-platform parity: identical semantic contracts on Win/macOS/Linux.
5. Small & auditable: each module stays focused; headers avoid template bloat.
6. “Borrow” before “own”: lightweight `slice` / `Str` views reduce copies.

---

## 🏁 Quick Start

Add SFC as a submodule or fetch + add_subdirectory:

```bash
git submodule add https://github.com/<yourfork>/sfc external/sfc
```

In your `CMakeLists.txt`:

```cmake
add_subdirectory(external/sfc)
target_link_libraries(your_app PRIVATE sfc)
```

Minimal example:

```cpp
#include "sfc/alloc.h"

using namespace sfc;

int main() {
    auto v = Vec<int>{};
    for(auto i = 0; i < 10; ++i) {
        v.push(i);
    }
    io::println("v[{}] = {}", v.len(), v);
    return 0;
}
```

---

## 🔬 Testing

Tests live next to sources (`*.cxx`) and are discovered via the single `sfc_test` runner.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j

# Windows
build\src\Debug\sfc_test.exe

# Unix
./build/src/sfc_test
```

Test expections use `sfc::expect_*` macros; a failure aborts fast with context.

---

## 🧩 Module Map (High Level)

```
alloc/        Box, Rc, String, Vec, allocation hooks
collections/  BTree, Hash, VecMap, queues
core/         Option, Result, iterators, future scaffolding, formatting traits
io/           File, stdio, error model
fs/           Path, file system wrappers
sync/         Mutex, Condvar, Atomics, MPMC queue
thread/       Thread creation & join primitives
time/         Clocks, durations
serde/        JSON, XML (early), serialization traits
log/          Core logging & pluggable backend structure
task/         Task primitives (emerging)
```

---

## ⚖️ Comparison Snapshot

| Concern | std:: | SFC |
|---------|-------|-----|
| Error handling | Return codes / exceptions | `Result<T,E>` + panic expects |
| Optional values | `std::optional` | `Option<T>` (match-like ergonomics) |
| Strings | `std::string` (allocator-heavy) | Explicit owned vs view types |
| Collections | Broad, legacy semantics | Purpose-built, STL-free, predictable |
| ABI / linkage | Vendor & version sensitive | Self-contained static library |
| Formatting | `<format>` heavy machinery | Lightweight evolving `fmt` traits |
| Hidden allocations | Possible in algorithms | Surfaced / explicit |

---

## 🗺️ Roadmap (Indicative)

* [ ] Richer formatting & compile-time format checking
* [ ] Arena / bump allocator
* [ ] Async executor + IO integration
* [ ] More zero-copy serde backends
* [ ] Configurable small-vector optimization
* [ ] Tracing + structured logging sink
* [ ] Additional lock-free data structures

Star or watch the repo to follow progress. PRs welcome (see below).

---

## 🤝 Contributing

We optimize for small, reviewable changes.

1. Fork & branch (`feat/xyz` or `fix/abc`).
2. Write tests next to the code (`something.cxx`).
3. Keep STL out (no `<vector>`, etc.).
4. Use existing patterns for memory & error handling.
5. Run test runner; ensure no regressions.
6. Open a PR with a concise rationale & benchmark notes if perf-related.

Please avoid large refactors without prior discussion (issue first).

---

## ❓ FAQ

**Q: Why GPLv3?**
Alignment with strong copyleft for foundational correctness; future dual-licensing may be discussed.

**Q: Can I use this in production today?**
Early stage; APIs may evolve. Lock a commit if shipping.

**Q: Does it replace the entire standard library?**
No. It offers a curated core; you can mix parts, but goal is self-sufficiency.

**Q: Exceptions?**
Design assumes exceptions are disabled or avoided; use `Result` + panics.

**Q: Allocator strategy?**
Central hooks allow future pluggable arenas and tracking.

---

## 🧪 Internal Testing Philosophy

Tests prefer deterministic, allocation-aware flows. Panics surface logic bugs early. Each collection targets: construction, boundary ops, iteration, error paths.

---

## 🔐 Safety Notes

Where raw pointers or unsafe casts are required, they’re isolated and documented. The public surface favors value semantics + explicit ownership.

---

## 🌐 Platform Support

| Platform | Status | Notes |
|----------|--------|-------|
| Windows (MSVC / ClangCL) | ✅ | Primary CI target |
| Linux (Clang/GCC) | ✅ | Glibc & musl intended |
| macOS (Clang) | ✅ | Unified APIs |
| arm64 | 🚧 | Most code is arch-agnostic; perf tuning later |

---

## 📦 Integration Tips

* Build static: no global runtime surprises.
* Disable exceptions / RTTI if you align project-wide.
* LTO + dead stripping can further shrink binary (std not linked).

---

## 📜 License

Distributed under GPL v3 – see [LICENSE](LICENSE). For commercial or alternative licensing discussions, open an issue.

---

## ⭐ Support & Momentum

If this vision resonates:

* Star to signal interest
* Watch for roadmap evolution
* Open issues for pain points / gaps
* Contribute focused PRs

Your feedback now directly shapes the direction before APIs harden.

---

## � Acknowledgements

Inspired by ideas proven in the Rust standard library ecosystem and other modern systems frameworks, reinterpreted for idiomatic C++23 without legacy ballast.

---

Happy hacking — unleash modern patterns without dragging all of libstd along.

— The SFC Project
