# Build System

This document explains how to configure, build, and test JAQL using CMake.
It covers all supported workflows: local development, IDE integration, CI, and
cross-platform builds.

---

## Prerequisites

Install all required tools before starting:

```bash
# CMake 3.25+
# On Ubuntu:
sudo apt install cmake ninja-build

# Optional: clang-format and clang-tidy for formatting and linting
sudo apt install clang-format-17 clang-tidy-17

# Conan 2 (requires Python 3.8+)
pip install conan
conan profile detect
```

Verify:

```bash
cmake --version    # must be ≥ 3.25
ninja --version    # must be ≥ 1.11
conan --version    # must be ≥ 2.0
```

---

## Dependency Installation with Conan 2

JAQL uses Conan 2 in [consumer workflow](https://docs.conan.io/2/) mode. All
dependencies will be declared in `conanfile.py` at the repository root.

### Step 1: Install dependencies

```bash
# From the repository root:
conan install . --build=missing -pr:b=default -pr:h=default
```

This resolves all dependencies, builds any that are not available as pre-built binaries,
and writes `conan_toolchain.cmake` and `conan_deps.cmake` into the build directory.

The `--build=missing` flag builds dependencies from source only when no compatible
binary is found in the cache or remote. On subsequent runs with the same compiler and
settings, binaries are served from the Conan cache — no recompilation.

### Step 2: Inspect the dependency graph

```bash
conan graph info . --format=text
```

### Conan Profiles

A Conan profile captures the compiler, settings, and options for a build. The default
profile created by `conan profile detect` is sufficient for most local development.

To create a profile for a specific configuration:

```bash
conan profile show default         # inspect the default profile
cp ~/.conan2/profiles/default ~/.conan2/profiles/clang17-debug
# Edit ~/.conan2/profiles/clang17-debug to set compiler=clang, version=17
```

For CI, the bootstrap step in the GitHub Actions workflow creates the profile
programmatically from environment variables.

---

## CMake Presets

JAQL uses `CMakePresets.json` for reproducible, IDE-integrated build configurations.
All available presets are listed in `CMakePresets.json`.

### Configure

```bash
cmake --preset <preset-name>
```

### Build

```bash
cmake --build --preset <preset-name>

# With parallelism
cmake --build --preset gcc-debug -- -j$(nproc)
```

### Test

```bash
ctest --preset <preset-name> --output-on-failure
```

### Available Presets

| Preset        | Build Type | Sanitizers      | Tests | Benchmarks | Extra Flags          |
|---------------|------------|-----------------|-------|------------|----------------------|
| `gcc-debug`   | Debug      | ASan + UBSan    | ON    | OFF        | Full debug info      |
| `gcc-release` | Release    | —               | ON    | OFF        | -O2, NDEBUG          |
| `ci-gcc-debug`    | Debug      | —               | ON    | OFF        | Coverage, -Werror    |
| `asan`        | Debug      | ASan            | ON    | OFF        |                      |
| `ubsan`       | Debug      | UBSan           | ON    | OFF        |                      |
| `tsan`        | Debug      | TSan            | ON    | OFF        | Incompatible with ASan |
| `benchmark`   | Release    | —               | OFF   | ON         | LTO, -O3, NDEBUG     |

### Build Directory Layout

Each preset writes to its own subdirectory of `build/`:

```
build/
├── gcc-debug/
│   ├── compile_commands.json    # used by clangd
│   └── ...
├── gcc-release/
├── ci-gcc-debug/
└── benchmark/
```

The `build/` directory is git-ignored. Never commit build artefacts.

---

## CMake Build Options

These options can be set on the command line with `-D<OPTION>=ON/OFF` or via a preset:

| Option                        | Default | Description                                      |
|-------------------------------|---------|--------------------------------------------------|
| `JAQL_BUILD_TESTS`            | `ON`    | Build the `tests/` target                        |
| `JAQL_BUILD_BENCHMARKS`       | `OFF`   | Build the `benchmarks/` target                   |
| `JAQL_BUILD_EXAMPLES`         | `OFF`   | Build the `examples/` target                     |
| `JAQL_ENABLE_ASAN`            | `OFF`   | Enable AddressSanitizer                          |
| `JAQL_ENABLE_UBSAN`           | `OFF`   | Enable UndefinedBehaviorSanitizer                |
| `JAQL_ENABLE_TSAN`            | `OFF`   | Enable ThreadSanitizer                           |
| `JAQL_ENABLE_COVERAGE`        | `OFF`   | Enable gcov/lcov coverage instrumentation        |
| `JAQL_ENABLE_CLANG_TIDY`      | `OFF`   | Run clang-tidy during the build                  |
| `JAQL_WARNINGS_AS_ERRORS`     | `OFF`   | Treat all warnings as errors (-Werror)           |

---

## Adding a New Module

New modules follow the `jaql_add_module()` helper defined in `cmake/JaqlModule.cmake`:

```cmake
# src/mymodule/CMakeLists.txt
jaql_add_module(
    mymodule
    SOURCES
        my_type.cpp
        my_algorithm.cpp
    HEADERS
        ../../include/jaql/mymodule/my_type.hpp
        ../../include/jaql/mymodule/my_algorithm.hpp
    DEPS
        core
        math
)
```

This creates:
- A library target `jaql_mymodule`
- An alias `jaql::mymodule` (use this in `target_link_libraries`)
- Correct include directories (`include/jaql/mymodule/` as public, `src/mymodule/detail/`
  as private)
- Compiler warning flags applied
- Sanitizer flags applied when the relevant preset is active
- Install rules for `cmake --install`

Then add `add_subdirectory(mymodule)` to `src/CMakeLists.txt` in topological order.

---

## Compiler Warnings

Warnings are configured in `cmake/CompilerWarnings.cmake` and applied to all targets by
`jaql_add_module()`. The warning set is aggressive by design — all warnings listed below
are enabled:

**GCC / Clang:**

```
-Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor -Wold-style-cast
-Wcast-align -Wunused -Woverloaded-virtual -Wconversion -Wsign-conversion
-Wnull-dereference -Wdouble-promotion -Wformat=2 -Wimplicit-fallthrough
```

**MSVC:** `/W4 /w14640 /w14265 /w14826 /w15038`

With `JAQL_WARNINGS_AS_ERRORS=ON` (used in CI): `-Werror` / `/WX` is added.

---

## Coverage Report

Run the `ci-gcc-debug` preset and then generate a coverage report:

```bash
cmake --preset ci-gcc-debug
cmake --build --preset ci-gcc-debug
ctest --preset ci-gcc-debug
cmake --build --preset ci-gcc-debug --target coverage
```

The HTML coverage report is written to `build/ci-gcc-debug/coverage/index.html`.

---

## IDE Integration

### VS Code

1. Install the [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)
   extension.
2. Open the repository folder. CMake Tools will detect `CMakePresets.json` automatically.
3. Select the `gcc-debug` preset from the status bar.
4. Install [clangd](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd).
   It uses `build/gcc-debug/compile_commands.json` automatically.

See `.vscode/settings.json` for the pre-configured workspace settings.

### CLion

1. Open the repository folder. CLion detects `CMakePresets.json` automatically
   (CLion 2023.2+).
2. Select `gcc-debug` as the active configuration.
3. Conan integration: install the [Conan plugin](https://plugins.jetbrains.com/plugin/11956-conan)
   or run `conan install .` before configuring — _once Conan integration is active_.

### compile_commands.json

All CMake configurations generate `compile_commands.json` in their build directory.
This file is required by clangd, `clang-tidy`, and include-what-you-use. A symlink at
the repository root can be created for convenience:

```bash
ln -sf build/gcc-debug/compile_commands.json compile_commands.json
```

---

## CI Pipeline

The GitHub Actions CI pipeline is defined in `.github/workflows/ci.yml`. It:

1. Checks out the repository.
2. Installs Conan 2 via `pip` and detects the default profile.
3. Runs `conan install . --build=missing`.
4. Configures CMake with the `ci-gcc-debug` preset.
5. Builds all targets.
6. Runs `ctest --output-on-failure`.
7. (On the GCC 13 Debug runner) uploads a coverage report to the CI artifacts.

The pipeline runs on every push to every branch.

---

## Developer Scripts

| Script                    | Purpose                                                  |
|---------------------------|----------------------------------------------------------|
| `scripts/bootstrap.sh`    | Install Conan deps and configure for development          |
| `scripts/format.sh`       | Run clang-format on all .hpp/.cpp files (in-place)       |
| `scripts/lint.sh`         | Run clang-tidy on all source files                       |
| `scripts/check_headers.sh`| Verify every public header compiles standalone           |

Run `scripts/bootstrap.sh` to get started after cloning. Then use the presets directly.
