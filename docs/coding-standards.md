# Coding Standards

JAQL follows the [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines) as its primary reference for code correctness, safety, and modern C++ patterns. Compliance is enforced in CI via `clang-tidy -checks=cppcoreguidelines-*`.

Formatting is handled separately by `clang-format` (see `.clang-format`).

The Core Guidelines deliberately do not prescribe naming conventions — those are defined
in the [Naming Conventions](#naming-conventions) section below.

---

## Tooling

| Concern | Tool | Config |
|---|---|---|
| Code correctness & safety | `clang-tidy` | `.clang-tidy` (`cppcoreguidelines-*`, `modernize-*`, `readability-*`) |
| Formatting | `clang-format` | `.clang-format` (Google style, column 100) |
| Self-contained headers | `scripts/check_headers.sh` | — |

All checks run in CI. VS Code (clangd) and CLion surface them live in the editor.

---

## Naming Conventions

| Entity | Convention | Example |
|---|---|---|
| Types, classes, concepts | `PascalCase` | `YieldCurve`, `Cashflow`, `Arithmetic` |
| Functions, methods | `snake_case` | `discount_factor()`, `build_curve()` |
| Variables, parameters | `snake_case` | `year_fraction`, `notional` |
| Private member variables | `snake_case_` (trailing `_`) | `settlement_date_` |
| Namespaces | `snake_case` | `jaql::rates` |
| `constexpr` constants | `snake_case` in namespace | `jaql::math::constants::pi` |
| `enum class` members | `PascalCase` | `DayCountConvention::Actual360` |
| Macros | `JAQL_SCREAMING_SNAKE` | `JAQL_ASSERT(...)` |
| Template type parameters | `PascalCase` | `template <typename Value>` |

**Functions use `snake_case`** to align with the C++ standard library (`std::find`,
`std::sort`) and the libraries JAQL builds on (Eigen, spdlog, nlohmann_json).

**Constants use `snake_case` in a namespace** to align with `std::numbers::pi`. The
namespace provides scoping; a `k`-prefix adds no information.

**Enum class members use `PascalCase`** because `enum class` scoping already prevents
name collisions — the `k` prefix was a workaround for unscoped enums only.

---

## Header Conventions

- **Extension:** `.hpp` (C++ headers), `.cpp` (source files). `.hpp` distinguishes
  C++ headers from C headers, matching Boost, Eigen, and most modern C++ libraries.
- **Include guard:** `#pragma once`. Simpler than `#ifndef` guards and supported by
  all target compilers (GCC 13+, Clang 17+, MSVC 2022).
- **Include order** (alphabetical within each group):
  1. Corresponding header (`.cpp` files only)
  2. Other JAQL headers
  3. Third-party headers (Eigen, spdlog, …)
  4. Standard library headers

---

## Forward Declarations

Forward declarations are allowed at inter-module boundaries when only a pointer or
reference is needed, to reduce compilation cascades. Only forward-declare types that
JAQL owns — never `std::` types or third-party types (consistent with Core Guideline
[SF.9](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#sf9-avoid-cyclic-dependencies-among-source-files)).

```cpp
namespace jaql::market { class MarketData; }

class PricingEngine {
    void attach(const MarketData* data);  // pointer only — full include not needed
};
```

---

## Error Handling

Functions that can fail in a recoverable, domain-meaningful way return `Result<T>`:

```cpp
using Result<T> = tl::expected<T, jaql::Error>;
```

- All `Result<T>`-returning functions are `[[nodiscard]]`.
- Propagate explicitly — never swallow errors silently.
- Chain operations with monadic operators (`.map()`, `.and_then()`).
- Never throw in a `noexcept` function.

---

## Strong Types

Use `StrongType<Tag, T>` for domain quantities to prevent unit errors at compile time:

```cpp
using Rate         = StrongType<struct RateTag,         double>;
using YearFraction = StrongType<struct YearFractionTag, double>;
using Notional     = StrongType<struct NotionalTag,     double>;
```

Zero runtime cost. The compiler enforces dimensional correctness.

---

## Template Constraints

Use C++23 Concepts (`requires`) to constrain template parameters. Do not use SFINAE
(`std::enable_if_t`). Prefer `requires(Condition)` syntax over `template <Concept T>`.

```cpp
// Good
template <typename T>
    requires std::floating_point<T>
auto lerp(T a, T b, T t) noexcept -> T;

// Bad — legacy SFINAE
template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
auto lerp(T a, T b, T t) noexcept -> T;
```
