# JAQL — Just A Quant Library

[![CI](https://github.com/jerarn/jaql/actions/workflows/ci.yml/badge.svg)](https://github.com/jerarn/jaql/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://isocpp.org/)
[![CMake 3.25+](https://img.shields.io/badge/CMake-3.25%2B-brightgreen.svg)](https://cmake.org/)

> A modern C++23 quantitative finance library engineered for correctness, performance,
> and long-term maintainability.

---

## Vision

JAQL is a modular C++ quantitative finance platform built for the long term — not a prototype. The architecture prioritizes **correctness first**, **explicitness over magic**, **testability**, and
**measurable performance**.

The library is designed to evolve into a complete quantitative finance stack:

- Instruments and cashflow framework
- Yield curve construction and term structure fitting
- Pricing library (vanilla and exotic derivatives)
- Risk analytics engine (Greeks, PnL attribution, scenario analysis)
- Monte Carlo simulation framework
- Calibration engine
- Portfolio analytics platform

---

## Architecture

JAQL is organized into three layers and ten modules with strict, unidirectional
dependencies (no cycles are ever permitted). See [Module Dependencies](docs/architecture/module-dependencies.md) for the full graph.

| Module        | Layer      | Responsibility                                             |
|---------------|------------|------------------------------------------------------------|
| `infra`       | Foundation | Logging, configuration, system services                    |
| `core`        | Foundation | Version, error handling (`Result<T>`), strong types        |
| `math`        | Foundation | Constants, numeric utilities, linear algebra abstractions  |
| `instruments` | Domain     | Financial instrument concepts, cashflow definitions        |
| `market`      | Domain     | Market data observables and quote types                    |
| `curves`      | Domain     | Yield curves, day count conventions, term structures       |
| `pricing`     | Analytics  | Pricing engine framework and result types                  |
| `simulation`  | Analytics  | Monte Carlo framework, scenario generation                 |
| `risk`        | Analytics  | Risk measures, sensitivities, stress testing               |
| `portfolio`   | Analytics  | Portfolio aggregation and analytics                        |

See [Architecture Overview](docs/architecture/overview.md) for the full design.

---

## Requirements

**Build tools:**

| Tool  | Minimum Version | Notes                            |
|-------|-----------------|----------------------------------|
| CMake | 3.25            | Required for `CMakePresets.json` |
| Ninja | 1.11            | Recommended generator            |

**Compiler — one of:**

| Compiler | Minimum Version | Platform | Notes                             |
|----------|-----------------|----------|-----------------------------------|
| GCC      | 13              | Linux    | Full C++23 support                 |
| Clang    | 17              | Linux    | Full C++23 + libc++ or libstdc++   |
| MSVC     | 2022 (17.0)     | Windows  | `/std:c++latest`                   |

**Primary platform:** Linux (Ubuntu 22.04+, Ubuntu 24.04+)
**Secondary platform:** Windows (MSVC 2022)
**Tested compilers:** See [CI matrix](.github/workflows/ci.yml)

---

## Quick Start

```bash
# Clone
git clone https://github.com/jerarn/jaql.git
cd jaql

# Install dependencies via Conan 2
conan install . --build=missing -pr:b=default -pr:h=default

# Configure — debug build with AddressSanitizer + UndefinedBehaviorSanitizer
cmake --preset gcc-debug

# Build
cmake --build --preset gcc-debug

# Run tests
ctest --preset gcc-debug --output-on-failure

# Run the example
./build/gcc-debug/examples/hello_jaql/hello_jaql
```

### Build Presets Reference

| Preset          | Purpose                           | Sanitizers   | Tests | Benchmarks |
|-----------------|-----------------------------------|--------------|-------|------------|
| `gcc-debug`    | Daily development                 | ASan + UBSan | ✓     | —          |
| `gcc-release`  | Optimized development             | —            | ✓     | —          |
| `ci-gcc-debug`      | CI pipeline (coverage + -Werror)  | —            | ✓     | —          |
| `asan`          | Memory safety validation          | ASan         | ✓     | —          |
| `ubsan`         | Undefined behaviour detection     | UBSan        | ✓     | —          |
| `tsan`          | Thread safety validation          | TSan         | ✓     | —          |
| `benchmark`     | Performance measurement (LTO)     | —            | —     | ✓          |

---

## Documentation

| Document                                                             | Description                                              |
|----------------------------------------------------------------------|----------------------------------------------------------|
| [Architecture Overview](docs/architecture/overview.md)               | System design, vision, module structure, extension points|
| [Design Principles](docs/architecture/design-principles.md)          | Engineering philosophy, ownership model, error handling  |
| [Module Dependencies](docs/architecture/module-dependencies.md)      | Full dependency graph and strict layering rules          |
| [Roadmap](docs/roadmap.md)                                           | Phase-by-phase development plan (Phases 0–4)             |
| [Tech Stack](docs/tech-stack.md)                                     | All tools, libraries, versions, and justifications       |
| [Coding Standards](docs/coding-standards.md)                         | Naming, headers, namespaces, templates, error handling   |
| [Testing Strategy](docs/testing-strategy.md)                         | Unit, integration, regression, benchmarking approach     |
| [Build System](docs/build-system.md)                                 | CMake presets, Conan 2 workflow, IDE integration         |
| [Contributing](CONTRIBUTING.md)                                      | Branch strategy, PR workflow, commit conventions         |

---

## Project Status

> **Phase 0 — Foundation** *(In Progress)*
>
> Establishing the build system, tooling, documentation, and module scaffolding.
> No pricing models are implemented yet. See the [Roadmap](docs/roadmap.md).

---

## License

MIT License — Copyright © 2026 Jeremy Arnaud. See [LICENSE](LICENSE) for full terms.
