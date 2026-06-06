# Build System

This document explains how to configure, build, and test JAQL using CMake.
It covers all supported workflows: local development, IDE integration, CI, and
cross-platform builds.

---

## Prerequisites

See [docs/dev/setup.md](dev/setup.md) for the full install procedure. Summary:

- CMake 3.25+, Ninja 1.11+, GCC 13+ or Clang 17+
- Conan 2.4+ (`pip install conan && conan profile detect`)

---

## Dependency Installation and Configure

JAQL uses Conan 2 in [consumer workflow](https://docs.conan.io/2/) mode with a
script-first local workflow.

### Recommended bootstrap flow

```bash
./scripts/bootstrap.sh
```

The bootstrap script installs Conan dependencies into the selected preset build
directory and then runs `cmake --preset <preset>`. By default it uses `gcc-debug`.

### Advanced bootstrap usage

```bash
# Alternate preset
./scripts/bootstrap.sh --preset clang-debug

# Explicit Conan host/build profiles (same model used in CI)
./scripts/bootstrap.sh --preset ci-gcc-debug --host-profile ci-gcc13 --build-profile ci-gcc13
```

### Inspect the dependency graph

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

| Preset             | Build Type | Sanitizers   | Tests | Benchmarks | Extra Flags               |
|--------------------|------------|--------------|-------|------------|---------------------------|
| `gcc-debug`        | Debug      | ASan + UBSan | ON    | OFF        | Full debug info           |
| `gcc-release`      | Release    | —            | ON    | OFF        | Optimized build           |
| `clang-debug`      | Debug      | ASan + UBSan | ON    | OFF        | Clang toolchain validation|
| `clang-release`    | Release    | —            | ON    | OFF        | Clang release validation  |
| `ci-gcc-debug`     | Debug      | —            | ON    | OFF        | Coverage, -Werror         |
| `ci-clang-debug`   | Debug      | —            | ON    | OFF        | -Werror                   |
| `ci-gcc-release`   | Release    | —            | ON    | OFF        | -Werror                   |
| `ci-clang-release` | Release    | —            | ON    | OFF        | -Werror                   |
| `asan`             | Debug      | ASan         | ON    | OFF        |                           |
| `ubsan`            | Debug      | UBSan        | ON    | OFF        |                           |
| `tsan`             | Debug      | TSan         | ON    | OFF        | Incompatible with ASan    |
| `benchmark`        | Release    | —            | OFF   | ON         | LTO                       |

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
| `JAQL_BUILD_DOCS`             | `OFF`   | Enable the `doxygen` documentation target        |

---

## Doxygen Pipeline

The repository includes a Doxygen template at `docs/Doxyfile.in` and an optional CMake
target enabled by passing `--docs` to bootstrap.

```bash
./scripts/bootstrap.sh --docs
cmake --build --preset gcc-debug --target doxygen
```

`--docs` installs Doxygen via Conan and injects `DOXYGEN_EXECUTABLE` and
`JAQL_BUILD_DOCS=ON` into the CMake cache automatically.

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
3. Run `./scripts/bootstrap.sh --preset gcc-debug` before first configure when Conan
   artifacts are missing.

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
2. Installs compiler/toolchain packages and Conan.
3. Creates explicit Conan profiles for each matrix entry.
4. Runs `./scripts/bootstrap.sh --preset <ci-preset> --host-profile <profile> --build-profile <profile>`.
5. Builds all targets with `cmake --build --preset <ci-preset>`.
6. Runs tests with `./scripts/test.sh --preset <ci-preset>`.
7. On the GCC 13 Debug CI job, conditionally generates and uploads coverage artifacts.

The pipeline runs on every push to every branch.

---

## Developer Scripts

| Script                    | Purpose                                                  |
|---------------------------|----------------------------------------------------------|
| `scripts/bootstrap.sh`    | Install Conan deps and configure for development          |
| `scripts/test.sh`         | Run ctest using the selected test preset                 |
| `scripts/format.sh`       | Run clang-format on all .hpp/.cpp files (in-place)       |
| `scripts/lint.sh`         | Run clang-tidy on all source files                       |
| `scripts/check_headers.sh`| Verify every public header compiles standalone           |

Run `scripts/bootstrap.sh` to get started after cloning. Then use the presets directly.
