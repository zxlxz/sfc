# SFC - C++ Common Library

  `SFC` is a fork of Rust's standard library, reimagined in modern C++20.

  It provides efficient static linking, lightweight code, and low runtime overhead, with strong fail-safe guarantees under correct usage.  
  SFC is designed for cross-platform support (Windows, macOS, Linux; x64/arm64) and does not depend on the C++ standard library.  
  The project aims for simple, maintainable code inspired by Rust's safety and modularity.

1. How to Build

- Ensure you have a C++20 compatible compiler (e.g. clang, gcc, MSVC).
- Run the following commands from the project root:
  ```bash
  cmake -B build
  cmake --build build
  ```

## 2. How to Test

- Unit tests are provided alongside source files as `*.cxx`.
- To run all tests:
  ```bash
  sfc-test
  ```
- Do not modify files under `src/sfc/test` (reserved for test library).

## 3. How to Contribute

- Fork the repository and create your feature branch.
- Submit pull requests with clear descriptions.
- All contributions are welcome! Join us to make SFC better.

## 4. License

- This project is licensed under the GNU GPL v3.
- See [LICENSE](LICENSE) for details.
