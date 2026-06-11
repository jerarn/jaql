# GitHub Copilot Instructions — JAQL

JAQL is a modular C++23 quantitative finance library. All generated code must conform
to the standards documented in `docs/`. This file is the authoritative quick-reference;
link to the relevant doc when fuller context is needed.

---

## Language and Compiler

- **Standard:** C++23. Use `std::expected`, `std::flat_map`, `std::print`, deducing
  `this`, and ranges improvements where appropriate.
- **Extensions:** OFF. `CXX_EXTENSIONS OFF` is enforced. Never use GNU or MSVC
  extensions — write strict ISO C++.
- **Target compilers:** GCC 13+, Clang 17+, MSVC 2022.

---

## Naming Conventions

| Entity                    | Convention                     | Example                          |
|---------------------------|--------------------------------|----------------------------------|
| Types, classes, concepts  | `PascalCase`                   | `YieldCurve`, `Arithmetic`       |
| Functions, methods        | `snake_case`                   | `discount_factor()`, `build_curve()` |
| Variables, parameters     | `snake_case`                   | `year_fraction`, `notional`      |
| Private member variables  | `snake_case_` (trailing `_`)   | `settlement_date_`               |
| Namespaces                | `snake_case`                   | `jaql::rates`                    |
| `constexpr` constants     | `snake_case` in namespace      | `jaql::math::constants::pi`      |
| `enum class` members      | `PascalCase`                   | `DayCountConvention::Actual360`  |
| Macros                    | `JAQL_SCREAMING_SNAKE`         | `JAQL_ASSERT(...)`               |
| Template type parameters  | `PascalCase`                   | `template <typename Value>`      |

---

## Error Handling

- Functions that can fail return `jaql::core::Result<T>` — an alias for `tl::expected<T, jaql::core::Error>`.
- All `Result<T>`-returning functions are `[[nodiscard]]`.
- Propagate errors explicitly — never swallow them silently.
- Chain with `.map()` and `.and_then()`.
- Never throw for domain-level failures (missing market data, out-of-range extrapolation,
  numerical convergence failure). Exceptions are reserved for unrecoverable programmer
  errors and constructor failures (use factory functions instead).

```cpp
// Correct pattern
[[nodiscard]] auto discount_factor(YearFraction t, Rate r) noexcept -> Result<double>;
```

---

## Strong Types

Use `StrongType<Tag, T>` for all domain quantities to prevent unit errors at compile time.
A `Rate` is never interchangeable with a `Spread`, even though both are `double`.

```cpp
using Rate         = StrongType<struct RateTag,         double>;
using YearFraction = StrongType<struct YearFractionTag, double>;
using Notional     = StrongType<struct NotionalTag,     double>;
```

---

## Template Constraints

Use C++23 Concepts with `requires`. Never use SFINAE (`std::enable_if_t`).

```cpp
// Good
template <typename T>
    requires std::floating_point<T>
auto lerp(T a, T b, T t) noexcept -> T;
```

---

## Header Conventions

- Extension: `.hpp` for C++ headers, `.cpp` for source files.
- Include guard: `#pragma once` only.
- Include order (alphabetical within each group):
  1. Corresponding header (`.cpp` files only)
  2. Other JAQL headers (`jaql/...`)
  3. Third-party headers (Eigen, spdlog, tl-expected, …)
  4. Standard library headers

---

## Formatting

- Style: Google, column limit 100, enforced by `clang-format` (config in `.clang-format`).
- Linting: `clang-tidy` with `cppcoreguidelines-*`, `modernize-*`, `readability-*`
  (config in `.clang-tidy`).

---

## Module Architecture

JAQL is organized into three layers. Dependencies are **one-way and strict** — lower
layers never depend on upper layers. Violations are build errors.

```
Foundation:  infra → core → math
Domain:      instruments, market, curves  (depend on Foundation)
Analytics:   pricing, simulation, risk, portfolio  (depend on Domain + Foundation)
```

Full dependency table: `docs/architecture/module-dependencies.md`

Public headers live under `include/jaql/<module>/`.
Source files live under `src/<module>/`.
Tests live under `tests/unit/<module>/` and `tests/integration/`.

---

## Testing

- Framework: GoogleTest + Google Mock.
- One test file per public header:
  `include/jaql/core/strong_type.hpp` → `tests/unit/core/test_strong_type.cpp`
- Test naming: `TEST(ClassName, MethodName_Scenario_ExpectedOutcome)`
- All `Result<T>` error paths are tested as explicitly as success paths.
- Never use `==` to compare `double`. Use `EXPECT_NEAR` or `JAQL_EXPECT_NEAR_REL`.
- Regression golden values are stored as `constexpr double` with a source comment.

---

## Commit Conventions

Follow Conventional Commits 1.0.0:

```
<type>(<scope>): <description>
```

Types: `feat`, `fix`, `perf`, `refactor`, `test`, `docs`, `build`, `ci`, `chore`
Scope: module name or infra area (`core`, `math`, `curves`, `cmake`, `ci`, …)
Rules: ≤72 chars on description line, imperative mood, each commit builds and tests pass.

---

## Key Documentation

| Topic                  | File                                          |
|------------------------|-----------------------------------------------|
| Coding standards       | `docs/coding-standards.md`                    |
| Architecture overview  | `docs/architecture/overview.md`               |
| Module dependencies    | `docs/architecture/module-dependencies.md`    |
| Design principles      | `docs/architecture/design-principles.md`      |
| Build system           | `docs/build-system.md`                        |
| Testing strategy       | `docs/testing-strategy.md`                    |
| Tech stack             | `docs/tech-stack.md`                          |
| Contribution workflow  | `docs/contributing.md`                        |
| Roadmap                | `docs/roadmap.md`                             |
