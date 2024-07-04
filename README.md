# SFC - A Modern C++ Standard Library Alternative

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Language: C++20](https://img.shields.io/badge/Language-C%2B%2B20-purple.svg)](https://isocpp.org/)
[![Platform: Windows | macOS | Linux](https://img.shields.io/badge/Platform-Windows%20%7C%20macOS%20%7C%20Linux-orange.svg)](https://isocpp.org/)

**SFC** is a foundational C++20 library designed from the ground up to be a modern, efficient, and safe alternative to the traditional C++ Standard Library. Inspired by the design of the Rust standard library, SFC provides a rich set of tools that emphasize compile-time safety, expressive APIs, and minimal overhead.

It is built to be entirely self-contained, with **zero dependencies on the C++ STL**, making it ideal for projects where binary size, performance, and control over the runtime are critical.

---

## ✨ Key Features

- **Rust-Inspired Ergonomics**: Enjoy safer, more expressive code with core types like `Result<T, E>` for robust error handling and `Option<T>` for optional values.
- **Performance-Oriented**: Designed for low-overhead abstractions and direct memory control, SFC helps you write high-performance applications without sacrificing readability.
- **Smart Memory Management**: Includes `Box<T>` (unique ownership) and `Rc<T>` (reference counting) for safer memory management patterns.
- **Rich Collection Types**: A powerful set of collections like `Vec<T>`, `VecDeque<T>`, `BTreeMap<K, V>`, and `HashMap<K, V>`.
- **Modern Concurrency**: Built-in tools for multithreading, including `Mutex`, `Condvar`, `Atomic`s, and a high-performance `Worker` thread pool for easy task scheduling.
- **Zero STL Dependency**: SFC is completely independent of `libstdc++` / `MSVCPRT`, giving you smaller binaries and full control over your application's environment.
- **Cross-Platform by Design**: A unified API for Windows, macOS, and Linux, abstracting away platform-specific details for the filesystem, I/O, and threading.
- **Simple Build & Integration**: Uses CMake for a straightforward, cross-platform build process.

---

## 🚀 Quick Example

Here’s a small example showcasing SFC's error handling and collection APIs.

```cpp
#include "sfc/alloc.h"
#include "sfc/io.h"

using namespace sfc;

// A function that can fail, returning a Result
auto parse_number(str::Str s) -> Result<int, str::Str> {
    auto num = s.parse<int>();
    if (num) {
        return *num;
    }
    return Str("Failed to parse number!");
}

int main() {
    // --- Using Vec ---
    auto my_vec = Vec<int>{};
    my_vec.push(1);
    my_vec.push(2);
    my_vec.push(3);
    my_vec.push(4);

    // Note: SFC provides basic I/O; integrated formatting is a work in progress.
    // The following is a conceptual example of how you might print it.
    io::print("Vector contents: ");
    for (const auto& item : my_vec.as_slice()) {
        io::print(" {}", item);
    }
    io::println("");


    // --- Using Result for error handling ---
    auto good_result = parse_number("123");
    io::print("Parsed number: {}", good_result.unwrap());

    auto bad_result = parse_number("abc");
    if (bad_result.is_err()) {
        io::print("Error: {}", bad_result.unwrap_err());
    }

    return 0;
}
```

---

## 🛠️ How to Build

Ensure you have a C++20 compatible compiler (e.g., Clang, GCC, or MSVC).

Run the following commands from the project root:
```bash
# Configure the project
cmake -B build

# Build the library and tests
cmake --build build
```

## 🧪 How to Test

Unit tests are provided alongside the source files (`*.cxx`). To run all tests after building, execute the test runner:

```bash
# On Windows
build\src\Debug\sfc_test.exe

# On Unix-like systems
./build/src/sfc_test
```

## ❤️ How to Contribute

Contributions are welcome! If you'd like to help improve SFC:
1.  Fork the repository.
2.  Create your feature branch.
3.  Submit a pull request with a clear description of your changes.

## 📜 License

This project is licensed under the **GNU GPL v3**. See the [LICENSE](LICENSE) file for details.
