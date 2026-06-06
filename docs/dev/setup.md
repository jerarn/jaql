# Developer Setup (Linux)

This guide is the fastest path to a working JAQL environment on Linux.

## Prerequisites

Install required tools:

```bash
sudo apt update
sudo apt install -y cmake ninja-build gcc-13 g++-13 clang-17 clang-format-17 clang-tidy-17 lcov python3-pip
python3 -m pip install --user --upgrade conan
conan profile detect --force
```

Verify toolchain versions:

```bash
cmake --version
ninja --version
conan --version
gcc-13 --version
clang-17 --version
```

## Bootstrap, Build, Test

From the repository root:

```bash
./scripts/bootstrap.sh
cmake --build --preset gcc-debug
./scripts/test.sh
```

Expected result:

- Configure succeeds for `gcc-debug`
- Build succeeds
- Tests pass

## Alternate Presets

```bash
./scripts/bootstrap.sh --preset clang-debug
cmake --build --preset clang-debug
./scripts/test.sh --preset clang-debug
```

## Optional: Generate API Documentation

```bash
./scripts/bootstrap.sh --docs
cmake --build --preset gcc-debug --target doxygen
```

`--docs` installs Doxygen via Conan and configures the build. Generated output is in `build/gcc-debug/docs/html/index.html`.
