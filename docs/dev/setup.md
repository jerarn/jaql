# Developer Setup (Linux)

This guide is the fastest path to a working JAQL environment on Linux.

## Prerequisites

Install required tools:

```bash
sudo apt update
sudo apt install -y cmake ninja-build gcc-13 g++-13 clang-17 clang-format-17 clang-tidy-17 libc++-17-dev libc++abi-17-dev lcov python3-pip
```

Install Conan 2 (required by `./scripts/bootstrap.sh`):

```bash
# Option A — quick start (works in any shell; ~/.local/bin must be on PATH)
python3 -m pip install --user 'conan>=2.4,<3'

# Option B — project virtualenv (recommended for VS Code/Cursor; .venv/ is gitignored)
python3 -m venv .venv
source .venv/bin/activate
pip install 'conan>=2.4,<3'
```

With option B, activate the venv (`source .venv/bin/activate`) in shells outside the
editor. VS Code/Cursor prepends `.venv/bin` to `PATH` in integrated terminals on Linux
(see [vscode-workflow.md](vscode-workflow.md)).

Bootstrap selects a Conan host profile from the CMake preset (see
[docs/build-system.md](../build-system.md)). No manual `conan profile detect` is required
unless you pass `--host-profile default`.

Conan downloads are cached in `.conan2/` at the repository root (configured by
[`.conanrc`](../.conanrc)). The directory is created on first bootstrap and is not
committed to git.

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

Clang presets use `profiles/ci/clang17-libcxx` (Clang 17 + libc++). Bootstrap sets
`CC`/`CXX` to `clang-17`/`clang++-17` automatically when they are unset, matching CI.

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

Verify documentation coverage (warnings are treated as errors):

```bash
./scripts/check_docs.sh
```
