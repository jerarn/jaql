---
agent: agent
description: Generate a GoogleTest unit test file for a given JAQL public header, following the testing strategy.
---

Generate a unit test file for the public header **`${input:headerPath}`**
(e.g. `include/jaql/core/strong_type.hpp`).

The test file lives at the mirrored path under `tests/unit/`:
`include/jaql/<module>/<header>.hpp` → `tests/unit/<module>/test_<header>.cpp`

Read the target header now to understand the types, functions, and failure modes
before generating any tests.

Also consult `docs/testing-strategy.md` for the full testing strategy.

---

## Structure to follow

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
    EXPECT_EQ(result.error().code(), Code::<ExpectedCode>);
}

// --- Compile-time invariants ---
// Use static_assert to document and enforce type constraints.
static_assert(!std::is_convertible_v<double, MyStrongType>,
    "<MyStrongType> must not be implicitly constructible from double");

} // namespace jaql::<module>::test
```

---

## Rules

1. **One test per logical behaviour**, not one test per function.
2. Naming: `TEST(ClassName, MethodName_Scenario_ExpectedOutcome)` — three underscore-separated parts.
3. Every `Result<T>` function must have at least one explicit error-path test.
4. Never use `EXPECT_EQ` or `==` to compare `double`. Use:
   - `EXPECT_NEAR(actual, expected, abs_tolerance)` for known absolute precision.
   - `JAQL_EXPECT_NEAR_REL(actual, expected, rel_tolerance)` for scale-varying results.
5. Use `static_assert` to verify compile-time type constraints (implicit conversion,
   move semantics, triviality, etc.).
6. Use `ASSERT_TRUE(result.has_value()) << result.error().message();` before dereferencing
   a `Result<T>` in a test — a failing `ASSERT` stops the test and prints the error.
7. No `using namespace` at file scope. Use the full `jaql::<module>::` prefix or
   a local `using` inside the test body.

---

## Checklist before finishing

- [ ] File placed at `tests/unit/<module>/test_<header>.cpp`.
- [ ] All public functions have at least one success-path test.
- [ ] All `Result<T>` error codes are covered by at least one test.
- [ ] No bare `==` comparisons on floating-point values.
- [ ] `static_assert` used to document compile-time invariants where applicable.
- [ ] Corresponding `CMakeLists.txt` updated to add the new `test_<header>.cpp` source
      (or a new `add_executable` if this is the first test in the module).
