---
name: jaql-new-test
description: Generate a GoogleTest unit test file for a given JAQL public header, following the testing strategy. Use when the user asks for unit tests, a test file for a header, or work under tests/unit/.
disable-model-invocation: true
---

# JAQL New Unit Test

Generate a unit test file for the public header the user provides
(e.g. `include/jaql/core/strong_type.hpp`).

The test file lives at the mirrored path under `tests/unit/`:
`include/jaql/<module>/<header>.hpp` → `tests/unit/<module>/test_<header>.cpp`

## Before Generating

1. Read the target header to understand types, functions, and failure modes.
2. Read [docs/testing-strategy.md](docs/testing-strategy.md) for the full testing strategy.

## Structure to Follow

```cpp
#include "jaql/<module>/<header>.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace jaql::<module>::test {

// --- Fixture (only if multiple tests share state) ---
class <ClassName>Test : public ::testing::Test {
protected:
    // shared setup
};

// --- Success paths ---
TEST_F(<ClassName>Test, <Method>_<Scenario>_<ExpectedOutcome>) {
    // Arrange
    // Act
    // Assert
}

// --- Error paths (one test per Result<T> failure mode) ---
TEST_F(<ClassName>Test, <Method>_<ErrorScenario>_ReturnsError) {
    auto result = /* call that should fail */;
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, Error::Code::<ExpectedCode>);
}

// --- Compile-time invariants ---
static_assert(!std::is_convertible_v<double, MyStrongType>,
    "<MyStrongType> must not be implicitly constructible from double");

} // namespace jaql::<module>::test
```

## Rules

1. **One test per logical behaviour**, not one test per function.
2. Naming: `TEST(ClassName, MethodName_Scenario_ExpectedOutcome)` — three underscore-separated parts.
3. Every `Result<T>` function must have at least one explicit error-path test.
4. Never use `EXPECT_EQ` or `==` to compare `double`. Use:
   - `EXPECT_NEAR(actual, expected, abs_tolerance)` for known absolute precision.
   - `JAQL_EXPECT_NEAR_REL(actual, expected, rel_tolerance)` for scale-varying results.
5. Use `static_assert` to verify compile-time type constraints.
6. Use `ASSERT_TRUE(result.has_value()) << result.error().message;` before dereferencing a `Result<T>`.
7. No `using namespace` at file scope.

## Checklist Before Finishing

- [ ] File placed at `tests/unit/<module>/test_<header>.cpp`.
- [ ] All public functions have at least one success-path test.
- [ ] All `Result<T>` error codes are covered by at least one test.
- [ ] No bare `==` comparisons on floating-point values.
- [ ] `static_assert` used where applicable.
- [ ] Corresponding `CMakeLists.txt` updated to add the new test source.
- [ ] Run `./scripts/test.sh` (or build the specific test target) to validate.
