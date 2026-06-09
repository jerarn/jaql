# Tech Stack

This document lists every tool, library, and service used in JAQL, with the exact
version, the reason it was chosen, and the alternatives that were considered.

Version pins are enforced in `conanfile.py` (libraries) and CI workflow files (tools).
Do not upgrade a dependency without updating this document and verifying the full test
suite.

---

## Language and Standard

| Item             | Version   | Notes                                                                          |
|------------------|-----------|--------------------------------------------------------------------------------|
| C++ standard     | **C++23** | `std::expected`, `std::flat_map`, `std::print`, deducing `this`, ranges improvements |
| `CXX_EXTENSIONS` | OFF       | No GNU/MSVC-specific extensions; strict ISO conformance                        |

---

## Compilers

| Compiler   | Minimum Version | Platform | Notes                                       |
|------------|-----------------|----------|---------------------------------------------|
| GCC        | 13              | Linux    | Full C++23; `-Wall -Wextra -Wpedantic`      |
| Clang      | 17              | Linux    | Full C++23; used for clang-tidy integration |
| MSVC       | 2022 (17.0)     | Windows  | `/std:c++latest /W4`                        |

### Rationale

GCC 13 and Clang 17 both provide full C++23 support including Concepts, `std::expected`,
`std::print`, and deducing `this`. MSVC 2022 provides sufficient C++23 support for
cross-platform builds. Earlier versions of these compilers have known C++23 conformance gaps.

---

## Build System

| Tool               | Version   | Purpose                                    |
|--------------------|-----------|--------------------------------------------|
| **CMake**          | 3.25+     | Build system generator                     |
| **Ninja**          | 1.11+     | Build backend (recommended over Make)      |
| **CMakePresets**   | v7        | Standardised, IDE-integrated build configs |

### Rationale

CMake 3.25 is the minimum required for `CMakePresets.json` schema version 7, which
supports `testPresets` and `buildPresets` in addition to `configurePresets`. Ninja is
preferred over GNU Make for significantly faster incremental builds on Linux.

---

## Dependency Management

| Tool        | Version | Purpose                                                |
|-------------|---------|--------------------------------------------------------|
| **Conan 2** | 2.4+    | C++ package manager; binary caching, reproducible deps |

### Rationale

See [ADR-0001](adr/ADR-0001-conan2-package-manager.md) for the full decision record.

**Summary:** Conan 2 was chosen over vcpkg and CMake FetchContent for its mature binary
caching, profile-based multi-platform builds, and wide adoption in institutional C++
shops. The `CMakeDeps` and `CMakeToolchain` generators provide clean CMake integration
with no lock-in.

### Conan 2 Key Commands

Prefer `./scripts/bootstrap.sh` for the full workflow. See [docs/build-system.md](build-system.md).

```bash
# Install dependencies (bootstrap does this automatically)
conan install . --build=missing \
  -pr:h=profiles/ci/gcc13 \
  -s build_type=Debug \
  --lockfile conan.lock --lockfile-partial

# List resolved dependency graph
conan graph info . -pr:h=profiles/ci/gcc13 -s build_type=Debug

# Search for a package version
conan search <package> -r conancenter
```

---

## Libraries

### Core Dependencies

| Library           | Version | Conan Ref              | Status | Purpose                                      |
|-------------------|---------|------------------------|--------|----------------------------------------------|
| **tl-expected**   | 1.2.0   | `tl-expected/1.2.0`    | Active | `Result<T>` — polyfill for `std::expected`   |
| **spdlog**        | 1.17.0  | `spdlog/1.17.0`        | Active | Structured logging with async support        |
| **Eigen**         | 5.0.1   | `eigen/5.0.1`          | Active | Linear algebra (matrices, vectors, solvers)  |
| **nlohmann_json** | 3.12.0  | `nlohmann_json/3.12.0` | Active | JSON config parsing and output serialization |

#### `tl-expected` vs `std::expected`

JAQL targets C++23, where `std::expected` is available natively. `tl-expected` is kept
as a dependency reference because it is interface-compatible and provides a fallback for
any compiler that has not yet shipped a conformant `std::expected`. If all target
compilers confirm full `std::expected` support before this dependency is activated, it
will be dropped in favour of the standard library type with no call-site changes.

### Testing and Quality

| Library              | Version | Conan Ref           | Status  | Purpose                      |
|----------------------|---------|---------------------|---------|------------------------------|
| **GoogleTest**       | 1.17.0  | `gtest/1.17.0`      | Active  | Unit and integration testing |
| **Google Benchmark** | 1.9.5   | `benchmark/1.9.5`   | Active  | Micro-benchmarking           |

---

## Tooling

### Code Quality

| Tool               | Version | Installation              | Purpose                                      |
|--------------------|---------|---------------------------|----------------------------------------------|
| **clang-format**   | 17+     | `apt install clang-format` | Automated code formatting                    |
| **clang-tidy**     | 17+     | `apt install clang-tidy`   | Static analysis and modernization linting     |
| **cppcheck**       | 2.13+   | `apt install cppcheck`     | Additional static analysis (optional, CI)    |

### Sanitizers

| Sanitizer                         | Flag                          | Preset  | Detects                              |
|-----------------------------------|-------------------------------|---------|--------------------------------------|
| AddressSanitizer (ASan)           | `-fsanitize=address`          | `asan`  | Heap/stack buffer overflows, use-after-free |
| UndefinedBehaviorSanitizer (UBSan)| `-fsanitize=undefined`        | `ubsan` | Integer overflow, null deref, misaligned access |
| ThreadSanitizer (TSan)            | `-fsanitize=thread`           | `tsan`  | Data races, lock-order violations    |
| LeakSanitizer (LSan)              | `-fsanitize=leak`             | (asan)  | Memory leaks (included with ASan on Linux) |

> **Note:** TSan is mutually exclusive with ASan/LSan. Use separate presets.

### Coverage

| Tool       | Version | Purpose                                       |
|------------|---------|-----------------------------------------------|
| **lcov**   | 1.16+   | Line/branch coverage collection from gcov     |
| **gcovr**  | 6.0+    | Alternative HTML/XML coverage reporter        |

---

## Documentation

| Tool          | Version | Purpose                                             |
|---------------|---------|-----------------------------------------------------|
| **Doxygen**   | 1.17+   | API documentation from `///` source comments        |
| **Graphviz**  | 9.0+    | Call graphs and dependency diagrams in Doxygen output|

The Doxygen configuration lives in [docs/Doxyfile.in](Doxyfile.in), processed by CMake.
Generated HTML and XML are produced in the local build directory via the `doxygen`
target. Automated publishing is not yet configured in CI.

Optional (future): Sphinx with Breathe extension for narrative documentation alongside
API docs.

---

## CI/CD

| Service              | Purpose                                              |
|----------------------|------------------------------------------------------|
| **GitHub Actions**   | CI pipeline (build, test, lint, coverage artifacts) |

### CI Runners

| Runner              | Compiler     | Used For                         |
|---------------------|--------------|----------------------------------|
| `ubuntu-24.04`      | GCC 13       | GCC debug/release CI and lint    |
| `ubuntu-24.04`      | Clang 17     | Clang debug/release CI           |

---

## Editor / IDE Support

| Tool                     | Recommended Extension                                   |
|--------------------------|---------------------------------------------------------|
| **VS Code**              | `ms-vscode.cmake-tools`, `llvm-vs-code-extensions.vscode-clangd` |
| **CLion**                | Built-in CMake and Conan integration (CLion 2024.1+)    |

### Language Server

[clangd](https://clangd.llvm.org/) is the recommended language server for both VS Code
and any LSP-compatible editor. It uses the `compile_commands.json` generated by CMake
(`CMAKE_EXPORT_COMPILE_COMMANDS=ON` is set in all presets).

---

## Alternatives Considered

| Category          | Chosen         | Alternatives Considered                | Reason Not Chosen                                 |
|-------------------|----------------|----------------------------------------|---------------------------------------------------|
| Package manager   | Conan 2        | vcpkg, CMake FetchContent              | See ADR-0001                                      |
| Testing           | GoogleTest     | Catch2 v3, doctest                     | GTest + GMock is the most widely known in quant C++; large test corpus exists |
| Logging           | spdlog         | Boost.Log, log4cpp                     | Header-only option, async support, zero overhead when disabled |
| Error handling    | tl-expected    | Outcome, Boost.Outcome, std::expected  | Lightest dependency, std::expected-compatible API |
| Linear algebra    | Eigen          | Blaze, Armadillo, MKL                  | Eigen is most widely used, well-documented, no BLAS dependency for initial use |
| JSON              | nlohmann_json  | RapidJSON, simdjson                    | Expressive API, sufficient performance for config; simdjson for perf-sensitive paths (Phase 4+) |
| Build backend     | Ninja          | GNU Make, Meson                        | Significantly faster than Make; simpler than Meson |
