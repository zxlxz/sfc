# SFC Copilot Instructions

Overview
- SFC: SFC is Rust stdlib fork in C++
- Target: C++23 only, no `<…>` headers or `std::*`
- Compiler: Clang≥18, GCC≥15;
- Platform: Win/Linux/macOS; x64/arm64

File conventions
- Header: `.h`
- Source: `.cc`
- Tests: `.cxx` next to code under test (never in `src/sfc/test`)

Copilot guidance
- Prefer SFC types/utilities; if missing, add minimal C++23-only impl
- Always skip standard library
- Keep tests in `.cxx` alongside code
- Add brief comments for SFC-specific patterns
- don't suggest about add-comments

UnitTest
- use SFC_TEST for unit tests
- use panic::assert for test assertions
```cpp
  namespace suite_name {
    SFC_TEST(case_name) {
      sfc::expect(cond, fmts...);
      panic::assert_eq(a, b);
      panic::assert_ne(a, b);
     }
  }
```
